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
#include <unistd.h>

static void close_cashier(shared_mem *, int);
static void dump_to_database(shared_mem *, int, int);

int main(int argc, char **argv) {
  // INIT VARS
  int service_time, break_time, shmid, i, parameters = 0, clients_left = 0;
  shared_mem *shared_mem_;
  srand(1);

  // READ ARGV INPUTS
  for (i = 1; i < argc - 1; i++) {
    if (argv[i][0] == '-' && argv[i][1] == 's') {
      service_time = atoi(argv[i + 1]);
      parameters++;
      // ERROR CHECK
      if (service_time < 0) {
        fprintf(stderr, "%s\n", "Incorrect Arguments Passed: Bad Service Time");
        exit(EXIT_FAILURE);
      }
    } else if (argv[i][0] == '-' && argv[i][1] == 'b') {
      break_time = atoi(argv[i + 1]);
      parameters++;
      // ERROR CHECK
      if (break_time < 0) {
        fprintf(stderr, "%s\n", "Incorrect Arguments Passed: Bad Break Time");
        exit(EXIT_FAILURE);
      }
    } else if (argv[i][0] == '-' && argv[i][1] == 'm') {
      shmid = atoi(argv[i + 1]);
      parameters++;
      // ERROR CHECK
      if (shmid < 0) {
        fprintf(stderr, "%s\n",
                "Incorrect Arguments Passed: Bad shmid reference");
        exit(EXIT_FAILURE);
      }
    }
  }
  if (parameters < 3) {
    fprintf(stderr, "%s\n", "Incorrect Arguments Passed: 3 Required Arguments");
    exit(EXIT_FAILURE);
  }

  // OPEN SHARED MEM AND SEMAPHORES
  shared_mem_ = attach_shared_mem(shmid);

  // OPEN SEMAPHORES
  shared_mem_->semaphores_.cashier_queue = sem_open(SEM_CASHIER_QUEUE, 0);
  shared_mem_->semaphores_.cashier_lock = sem_open(SEM_CASHIER_LOCK, 0);
  shared_mem_->semaphores_.client_cashier = sem_open(SEM_CLIENT_CASHIER, 0);
  shared_mem_->semaphores_.client_queue = sem_open(SEM_CLIENT_QUEUE, 0);

  // CHECK-IN : DECREMENT CASHIERS COUNTER IF NOT FULL
  if (sem_wait(shared_mem_->semaphores_.cashier_lock) == -1) { // acquire lock
    perror("sem_t CASHIER_LOCK wait failed");
    close_cashier(shared_mem_, shmid);
  }
  if (shared_mem_->counters_.cashier > 0) {
    clients_left = --shared_mem_->counters_.client;
    printf("Cashier %d is now serving clients\n",
           clients_left + 1 - shared_mem_->counters_.max_cashier);
  } else {
    close_cashier(shared_mem_, shmid);
  }
  if (sem_post(shared_mem_->semaphores_.cashier_lock) == -1) { // release lock
    perror("sem_t CASHIER_LOCK post failed");
    close_cashier(shared_mem_, shmid);
  }

  while (clients_left > 0) {
    VLOG(DEBUG, "BEFORE LOCK");
    if (sem_wait(shared_mem_->semaphores_.cashier_lock) == -1) { // acquire lock
      perror("sem_t CASHIER_LOCK wait in loop failed");
      close_cashier(shared_mem_, shmid);
    }
    VLOG(DEBUG, "AFTER LOCK");
    if (sem_trywait(shared_mem_->semaphores_.client_queue) == -1) {
      if (errno == EAGAIN) {
        VLOG(DEBUG, "HERE=IN");
        // TAKE A BREAK, no one in client queue
        printf("Cashier is breaking for... %ds\n", break_time);
        sleep(break_time);
      } else {
        perror("sem_t CLIENT_QUEUE trywait failed");
        close_cashier(shared_mem_, shmid);
      }
    } else { // FOUND A WAITING CLIENT
      VLOG(DEBUG, "HERE=OUT");
      if (sem_post(shared_mem_->semaphores_.cashier_queue) == -1) {
        perror("sem_t CASHIER_QUEUE post failed");
        close_cashier(shared_mem_, shmid);
      }
      // TAKING ORDER -- SERVICE TIME
      int this_service_time = randomize_n(service_time);
      int cur_client = MAX_CLIENTS - shared_mem_->counters_.client;
      shared_mem_->clients[cur_client].cashier_time = this_service_time;
      // WAIT TILL CLIENT ADDS ORDER
      if (sem_wait(shared_mem_->semaphores_.client_cashier) == -1) {
        perror("sem_t CLIENT_CASHIER wait in loop failed");
        close_cashier(shared_mem_, shmid);
      }
      if (sem_post(shared_mem_->semaphores_.cashier_lock) ==
          -1) { // release lock
        perror("sem_t CASHIER_LOCK post in loop failed");
        close_cashier(shared_mem_, shmid);
      }
      printf("Cashier serving client %d (%d) in... %ds",
             shared_mem_->clients[cur_client].client_id, cur_client,
             this_service_time);
      sleep(this_service_time);
      int order = shared_mem_->clients[i].item_id;
      shared_mem_->menu[order].quantity++;
      dump_to_database(shared_mem_, cur_client, order);
    }
    clients_left = shared_mem_->counters_.client;
  }

  // DETACH SHARED MEM AND CLOSE SEMAPHORES
  detach_shared_mem(shared_mem_, shmid);
  sem_close(shared_mem_->semaphores_.cashier_queue);
  sem_close(shared_mem_->semaphores_.cashier_lock);
  sem_close(shared_mem_->semaphores_.client_cashier);
  sem_close(shared_mem_->semaphores_.client_queue);

  return EXIT_SUCCESS;
}

void close_cashier(shared_mem *shared_mem_, int shmid) {

  detach_shared_mem(shared_mem_, shmid);
  sem_close(shared_mem_->semaphores_.cashier_queue);
  sem_close(shared_mem_->semaphores_.cashier_lock);
  sem_close(shared_mem_->semaphores_.client_cashier);
  sem_close(shared_mem_->semaphores_.client_queue);

  exit(EXIT_FAILURE);
}

void dump_to_database(shared_mem *shared_mem_, int i, int order) {
  FILE *fp;
  fp = fopen(database_dir, "a");
  if (fp == NULL) {
    perror("Failed to Append Client Data to Database");
  } else {
    fprintf(fp, "%d-%d, %s, %f\n", i, shared_mem_->clients[i].client_id,
            shared_mem_->menu[order].description,
            shared_mem_->menu[order].price);
  }
  fclose(fp);
}
