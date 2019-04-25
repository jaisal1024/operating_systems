#include "common.h"
#include "func.h"
#include <ctype.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <time.h>
#include <unistd.h>

extern char *tzname[2];

static void sort_by_quantity(shared_mem *, int, int *);
static int find_min(int *, shared_mem *);

int main(int argc, char **argv) {
  // INITIALIZE VARS
  int shm_id, i, max_num_cashiers, parameters = FAILURE, menu_index = 0,
                                   no_of_clients_served;

  double avg_waiting_time, revenue, avg_time_in_shop;
  semaphores semaphores_;
  time_t time_;
  time(&time_);
  srand(1);
  FILE *fp, *fp_out;
  char *ln = NULL;
  size_t cap = MAX_INPUT_SIZE;
  ssize_t ln_size;
  const char *delim = ",";
  char *token;
  const char *menu_dir = "menu.txt";
  const char *output_dir = "daily_stats";
  shared_mem *shared_mem_;

  // READ ARGV INPUTS
  for (i = 1; i < argc - 1; i++) {
    if (argv[i][0] == '-' && argv[i][1] == 'n') {
      max_num_cashiers = atoi(argv[i + 1]);
      // ERROR CHECK
      if (max_num_cashiers > 0) {
        parameters = SUCCESS;
        break;
      }
    }
  }
  if (parameters == FAILURE) {
    fprintf(stderr, "%s\n", "Incorrect Arguments Passed");
    exit(EXIT_FAILURE);
  }

  // SHARED MEMORY CREATION
  shm_id = shmget(IPC_PRIVATE, sizeof(shared_mem), IPC_CREAT | RW_ACCESS);
  if (shm_id == -1) {
    perror("Create Shared Memory Failed");
    exit(EXIT_FAILURE);
  }

  shared_mem_ = (shared_mem *)shmat(shm_id, NULL, 0);
  if (*(int *)shared_mem_ == -1) {
    perror("Attach Shared Memory Failed");
    exit(EXIT_FAILURE);
  }
  printf("Shared memory allocated at: %d\n", shm_id);

  // SET CLIENT_COUNT to 0
  shared_mem_->counters_.client = MAX_CLIENTS;
  shared_mem_->counters_.client_queue = MAX_QUEUE;
  shared_mem_->counters_.server = 1;
  shared_mem_->counters_.cashier = max_num_cashiers;
  shared_mem_->counters_.max_cashier = max_num_cashiers;
  // SET SERVER TIME TO 0
  shared_mem_->server_time = 0;

  // CREATE STRUCT, READ MENU
  fp = fopen(menu_dir, "r");
  if (fp == NULL) {
    perror("Menu Failed to Load");
    exit(EXIT_FAILURE);
  }
  while ((ln_size = getline(&ln, &cap, fp)) > 0) {
    int j = 0;
    while ((token = strsep(&ln, delim)) != NULL) {
      if (*token != '\0') {
        switch (j++) {
        case ITEM_ID:
          shared_mem_->menu[menu_index].item_id = atoi(token);
          break;
        case DESCR:
          strncpy(shared_mem_->menu[menu_index].description, token,
                  strlen(token));
          break;
        case PRICE:
          shared_mem_->menu[menu_index].price = atof(token);
          break;
        case MIN_TIME:
          shared_mem_->menu[menu_index].min_time = atoi(token);
          break;
        case MAX_TIME:
          shared_mem_->menu[menu_index].max_time = atoi(token);
          break;
        default:
          fprintf(stderr, "Too many arguments passed in menu file\n");
          break;
        }
      }
    }
    shared_mem_->menu[menu_index].quantity = 0;
    menu_index++;
  }
  printf("Loaded %d Menu Items from ~/%s\n", menu_index, menu_dir);
  fclose(fp);

  // INIT SEMAPHORES
  semaphores_.manager_lock =
      sem_open(SEM_MANAGER_LOCK, O_CREAT | O_EXCL, RW_ACCESS, 0);
  if (semaphores_.manager_lock == SEM_FAILED) {
    perror("Sem_t MANAGER_LOCK Open Failed");
    exit(EXIT_FAILURE);
  }

  semaphores_.cashier_queue =
      sem_open(SEM_CASHIER_QUEUE, O_CREAT | O_EXCL, RW_ACCESS, 0);
  if (semaphores_.cashier_queue == SEM_FAILED) {
    perror("Sem_t CASHIER_QUEUE Open Failed");
    exit(EXIT_FAILURE);
  }

  semaphores_.cashier_lock =
      sem_open(SEM_CASHIER_LOCK, O_CREAT | O_EXCL, RW_ACCESS, 1);
  if (semaphores_.cashier_lock == SEM_FAILED) {
    perror("Sem_t CASHIER_LOCK Open Failed");
    exit(EXIT_FAILURE);
  }

  semaphores_.client_cashier =
      sem_open(SEM_CLIENT_CASHIER, O_CREAT | O_EXCL, RW_ACCESS, 0);
  if (semaphores_.client_cashier == SEM_FAILED) {
    perror("Sem_t CLIENT_CASHIER Open Failed");
    exit(EXIT_FAILURE);
  }

  semaphores_.client_queue =
      sem_open(SEM_CLIENT_QUEUE, O_CREAT | O_EXCL, RW_ACCESS, 0);
  if (semaphores_.client_queue == SEM_FAILED) {
    perror("Sem_t CLIENT_QUEUE Open Failed");
    exit(EXIT_FAILURE);
  }

  semaphores_.client_lock =
      sem_open(SEM_CLIENT_LOCK, O_CREAT | O_EXCL, RW_ACCESS, 1);
  if (semaphores_.client_lock == SEM_FAILED) {
    perror("Sem_t CLIENT LOCK Open Failed");
    exit(EXIT_FAILURE);
  }

  semaphores_.client_server =
      sem_open(SEM_CLIENT_SERVER, O_CREAT | O_EXCL, RW_ACCESS, 0);
  if (semaphores_.client_server == SEM_FAILED) {
    perror("Sem_t CLIENT_SERVER Open Failed");
    exit(EXIT_FAILURE);
  }

  semaphores_.server_queue = sem_open(SEM_SERVER_QUEUE, O_CREAT, RW_ACCESS, 0);
  if (semaphores_.server_queue == SEM_FAILED) {
    perror("Sem_t SERVER_QUEUE Open Failed");
    exit(EXIT_FAILURE);
  }

  semaphores_.server_lock =
      sem_open(SEM_SERVER_LOCK, O_CREAT | O_EXCL, RW_ACCESS, 1);
  if (semaphores_.server_lock == SEM_FAILED) {
    perror("Sem_t SERVER_LOCK Open Failed");
    exit(EXIT_FAILURE);
  }

  // OPEN RESTO and PUT MANAGER TO SLEEP
  printf("The Restaurant is now open at: %s", ctime(&time_));

  // PRINT TIMESTAMP ONTO DATABSE
  fp = fopen(database_dir, "a");
  if (fp == NULL) {
    perror("Database Failed to Open");
  } else {
    fprintf(fp, "%s", ctime(&time_));
  }
  fclose(fp);

  // PUT TO SLEEP TILL WOKEN UP
  if (sem_wait(semaphores_.manager_lock) == -1) {
    perror("sem_t MANAGER_LOCK wait failed");
    detach_shared_mem_and_close_all_sem(shared_mem_, shm_id, semaphores_);
    exit(EXIT_FAILURE);
  }

  // COMPILE & PRINT STATS
  avg_waiting_time = 0;
  avg_time_in_shop = 0;
  no_of_clients_served = 0;
  revenue = 0;
  // OUPUT TO DATABASE && OUPUT FILE
  fp_out = fopen(output_dir, "w");
  if (fp_out == NULL) {
    perror("Daily outputs file Failed to Open");
  } else {
    double wait_time, spending, time_in_shop;
    for (i = 0; i < MAX_CLIENTS - shared_mem_->counters_.client; i++) {
      time_in_shop = shared_mem_->clients[i].depart_time -
                     shared_mem_->clients[i].arrival_time;
      wait_time = shared_mem_->clients[i].cashier_time +
                  shared_mem_->clients[i].food_time +
                  shared_mem_->clients[i].server_time;
      spending = shared_mem_->clients[i].bill;

      fprintf(fp_out,
              "Client %d spent %f s in the shop waited %f s and spent $ %f\n",
              shared_mem_->clients[i].client_id, time_in_shop, wait_time,
              spending);
      no_of_clients_served++;
      avg_time_in_shop += time_in_shop;
      avg_waiting_time += wait_time;
      revenue += spending;
    }
    avg_time_in_shop /= no_of_clients_served;
    avg_waiting_time /= no_of_clients_served;
    fprintf(fp, "Avg. Waiting Time: %f s\n", avg_waiting_time);
    fprintf(fp, "Avg. Time in the Shop: %f s\n", avg_time_in_shop);

    fprintf(fp, "Top 5 Menu Sales\n");
    int top_5[5] = {0, 1, 2, 3, 4};
    sort_by_quantity(shared_mem_, menu_index, &top_5[0]);
    for (i = 0; i < 5; i++) {
      if (shared_mem_->menu[top_5[i]].quantity > 0) {
        VLOG(DEBUG, "top_5: %d", top_5[i]);
        fprintf(fp, "%s : $ %f\n", shared_mem_->menu[top_5[i]].description,
                shared_mem_->menu[top_5[i]].price *
                    shared_mem_->menu[top_5[i]].quantity);
      }
    }

    fprintf(fp, "No. of Clients Served: %d\n", no_of_clients_served);
    fprintf(fp, "Total Revenue: $ %f\n", revenue);
  }
  fclose(fp_out);

  // print statistics of the day
  fp = fopen(database_dir, "a");
  if (fp == NULL) {
    perror("Database Failed to Open");
  } else {
    fprintf(fp, "Avg. Waiting Time: %f\n", avg_waiting_time);
    fprintf(fp, "Avg. Time in the Shop: %f\n", avg_time_in_shop);
    fprintf(fp, "No. of Clients Served: %d\n", no_of_clients_served);
    fprintf(fp, "Total Revenue: %f\n", revenue);
  }
  fclose(fp);

  // GRACEFULLY EXIT
  detach_shared_mem_and_close_all_sem(shared_mem_, shm_id, semaphores_);
  printf("See you tomorrow! \n");

  return EXIT_SUCCESS;
}

void sort_by_quantity(shared_mem *shared_mem_, int menu_max, int *top_5) {
  int i;
  int min_index = find_min(&top_5[0], shared_mem_);
  for (i = 5; i < menu_max; i++) {
    if (shared_mem_->menu[i].quantity > shared_mem_->menu[min_index].quantity) {
      top_5[min_index] = i;
      find_min(&top_5[0], shared_mem_);
    }
  }
}

int find_min(int *top_5, shared_mem *shared_mem_) {
  int i;
  int min_index = 0;
  for (i = 1; i < 5; i++) {
    if (shared_mem_->menu[top_5[i]].quantity <
        shared_mem_->menu[top_5[min_index]].quantity)
      min_index = i;
  }
  return min_index;
}
