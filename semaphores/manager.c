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

int main(int argc, char **argv) {
  // INITIALIZE VARS
  int shm_id, i, max_num_cashiers, parameters = FAILURE, menu_index = 0,
                                   avg_waiting_time, no_of_clients_served;
  time_t time_;
  srand(1);
  FILE *fp;
  char *ln = NULL;
  size_t cap = MAX_INPUT_SIZE;
  ssize_t ln_size;
  const char *delim = ",";
  char *token;
  const char *menu_dir = "menu.txt";
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
  shared_mem_->counters_.server = 1;
  shared_mem_->counters_.cashier = max_num_cashiers;

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
  fclose(fp);

  // INIT SEMAPHORES
  shared_mem_->semaphores_.manager_lock =
      sem_open(SEM_MANAGER_LOCK, O_CREAT, RW_ACCESS, 0);
  if (shared_mem_->semaphores_.manager_lock == SEM_FAILED) {
    perror("Sem_t MANAGER_LOCK Open Failed");
    exit(EXIT_FAILURE);
  }

  shared_mem_->semaphores_.cashier_queue =
      sem_open(SEM_CASHIER_QUEUE, O_CREAT, RW_ACCESS, 0);
  if (shared_mem_->semaphores_.cashier_queue == SEM_FAILED) {
    perror("Sem_t CASHIER_QUEUE Open Failed");
    exit(EXIT_FAILURE);
  }

  shared_mem_->semaphores_.cashier_lock =
      sem_open(SEM_CASHIER_LOCK, O_CREAT, RW_ACCESS, 1);
  if (shared_mem_->semaphores_.cashier_lock == SEM_FAILED) {
    perror("Sem_t CASHIER_LOCK Open Failed");
    exit(EXIT_FAILURE);
  }

  shared_mem_->semaphores_.client_cashier =
      sem_open(SEM_CLIENT_CASHIER, O_CREAT, RW_ACCESS, 0);
  if (shared_mem_->semaphores_.client_cashier == SEM_FAILED) {
    perror("Sem_t CLIENT_CASHIER Open Failed");
    exit(EXIT_FAILURE);
  }

  shared_mem_->semaphores_.client_queue =
      sem_open(SEM_CLIENT_QUEUE, O_CREAT, RW_ACCESS, 0);
  if (shared_mem_->semaphores_.client_queue == SEM_FAILED) {
    perror("Sem_t CLIENT_QUEUE Open Failed");
    exit(EXIT_FAILURE);
  }

  shared_mem_->semaphores_.client_server =
      sem_open(SEM_CLIENT_SERVER, O_CREAT, RW_ACCESS, 0);
  if (shared_mem_->semaphores_.client_server == SEM_FAILED) {
    perror("Sem_t CLIENT_SERVER Open Failed");
    exit(EXIT_FAILURE);
  }

  shared_mem_->semaphores_.server_queue =
      sem_open(SEM_SERVER_QUEUE, O_CREAT, RW_ACCESS, 0);
  if (shared_mem_->semaphores_.server_queue == SEM_FAILED) {
    perror("Sem_t SERVER_QUEUE Open Failed");
    exit(EXIT_FAILURE);
  }

  shared_mem_->semaphores_.server_lock =
      sem_open(SEM_SERVER_LOCK, O_CREAT, RW_ACCESS, 1);
  if (shared_mem_->semaphores_.server_lock == SEM_FAILED) {
    perror("Sem_t SERVER_LOCK Open Failed");
    exit(EXIT_FAILURE);
  }

  // OUPUT TO DATABASE
  fp = fopen(database_dir, "a");
  if (fp == NULL) {
    perror("Database Failed to Open");
  } else {
    // print time of day in ASCII
    time(&time_);
    fprintf(fp, "%s", ctime(&time_));
  }
  fclose(fp);

  // PUT TO SLEEP TILL WOKEN UP
  if (sem_wait(shared_mem_->semaphores_.manager_lock) == -1) {
    perror("sem_t MANAGER_LOCK wait failed");
    detach_shared_mem_and_close_all_sem(shared_mem_, shm_id);
    exit(EXIT_FAILURE);
  }

  // COMPILE & PRINT STATS
  avg_waiting_time = 0;
  no_of_clients_served = 0;

  // print statistics of the day
  fp = fopen(database_dir, "a");
  if (fp == NULL) {
    perror("Database Failed to Open");
  } else {
    fprintf(fp, "Avg. Waiting Time: %d\n", avg_waiting_time);
    fprintf(fp, "No. of Clients Served: %d\n\n", no_of_clients_served);
  }
  fclose(fp);

  // GRACEFULLY EXIT
  detach_shared_mem_and_close_all_sem(shared_mem_, shm_id);
  printf("See you tomorrow! \n");

  return EXIT_SUCCESS;
}
