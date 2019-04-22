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
  int shm_id, i, max_num_cashiers, parameters = FAILURE, menu_index = 0, status,
                                   avg_waiting_time, no_of_clients_served;
  struct tm *timeptr = NULL;
  void *mem_ptr;
  FILE *fp;
  char *ln = NULL;
  size_t cap = MAX_INPUT_SIZE;
  ssize_t ln_size;
  const char *delim = ",";
  char *token;
  // const char *mem_name = "SHARED_MEM";
  const char *menu_dir = "menu.txt";
  const char *database_dir = "database.txt";
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
  shm_id = shmget(IPC_PRIVATE, sizeof(shared_mem), IPC_CREAT | 0666);
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
  shared_mem_->sem_cashiers =
      sem_open("/SEM_CASHIER", O_CREAT, 0666, max_num_cashiers);
  VLOG(DEBUG, "%d", shared_mem_->sem_cashiers);
  if (shared_mem_->sem_cashiers == SEM_FAILED) {
    perror("Sem Cashier Open Failed");
    exit(EXIT_FAILURE);
  }
  shared_mem_->sem_clients =
      sem_open("/SEM_CLIENT", O_CREAT, 0666, MAX_CLIENTS_SERVED);
  if (shared_mem_->sem_clients == SEM_FAILED) {
    perror("Sem Clients Open Failed");
    exit(EXIT_FAILURE);
  }
  shared_mem_->sem_binary_locking = sem_open("/SEM_LOCK", O_CREAT, 0666, 1);
  if (shared_mem_->sem_binary_locking == SEM_FAILED) {
    perror("Sem Locking Open Failed");
    exit(EXIT_FAILURE);
  }

  // ATTACH TO SHARED MEMORY
  // mem_ptr = shared_mem_;
  VLOG(DEBUG, "Menu 1: %s", shared_mem_->menu[0].description);

  // PUT TO SLEEP TILL WOKEN UP
  if (sem_wait(shared_mem_->sem_clients) == -1) {
    perror("Sem Client Wait Failed");
    exit(EXIT_FAILURE);
  }

  // COMPILE & PRINT STATS
  avg_waiting_time = 0;
  no_of_clients_served = 0;

  // OUPUT TO DATABASE
  fp = fopen(database_dir, "a");
  if (fp == NULL) {
    perror("Database Failed to Open");
    exit(EXIT_FAILURE);
  }
  // print time of day in ASCII
  timelocal(timeptr);
  char *ascii_time = asctime(timeptr);
  fprintf(fp, "%s\n", ascii_time);
  // print menu orders of the day - item_id, description, quantity
  for (i = 0; i < MENU_SIZE; i++) {
    if (shared_mem_->menu[i].quantity > 0) {
      fprintf(fp, "%d, %s, %d, \n", shared_mem_->menu[i].item_id,
              shared_mem_->menu[i].description, shared_mem_->menu[i].quantity);
    }
  }
  // print statistics of the day
  fprintf(fp, "Avg. Waiting Time: %d\n", avg_waiting_time);
  fprintf(fp, "No. of Clients Served: %d\n", no_of_clients_served);

  // GRACEFULLY EXIT

  return EXIT_SUCCESS;
}
