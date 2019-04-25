#include "common.h"
#include "func.h"
#include <ctype.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <unistd.h>

static void close_server(shared_mem *, int);

int main(int argc, char **argv) {
  // INIT VARS
  int i, shmid, parameters = 0, clients_left = 0;
  shared_mem *shared_mem_;

  // READ ARGV INPUTS
  for (i = 1; i < argc - 1; i++) {
    if (argv[i][0] == '-' && argv[i][1] == 'm') {
      shmid = atoi(argv[i + 1]);
      parameters++;
      // ERROR CHECK
      if (shmid < 0) {
        fprintf(stderr, "%s\n",
                "Incorrect Arguments Passed: Bad shmid reference");
        exit(EXIT_FAILURE);
      } else
        break;
    }
  }
  if (parameters < 1) {
    fprintf(stderr, "%s\n", "Incorrect Arguments Passed");
    exit(EXIT_FAILURE);
  }

  // OPEN SHARED MEM AND SEMAPHORES
  shared_mem_ = attach_shared_mem(shmid);

  // OPEN SEMAPHORES
  shared_mem_->semaphores_.server_queue = sem_open(SEM_SERVER_QUEUE, 0);
  shared_mem_->semaphores_.server_lock = sem_open(SEM_SERVER_LOCK, 0);
  shared_mem_->semaphores_.client_server = sem_open(SEM_CLIENT_SERVER, 0);

  VLOG(DEBUG, "INIT SEMS: %d", shared_mem_->semaphores_.server_queue);

  // CHECK-IN : DECREMENT SERVER COUNTER IF NOT FULL
  if (sem_wait(shared_mem_->semaphores_.server_lock) == -1) { // acquire lock
    perror("sem_t SERVER_LOCK wait failed");
    close_server(shared_mem_, shmid);
  }
  if (shared_mem_->counters_.server > 0) {
    shared_mem_->counters_.server--;
    clients_left = shared_mem_->counters_.client;
  } else {
    fprintf(stderr, "Server already on the job\n");
    if (sem_post(shared_mem_->semaphores_.server_lock) == -1) { // release lock
      perror("sem_t SERVER_LOCK post failed");
      close_server(shared_mem_, shmid);
    }
    close_server(shared_mem_, shmid);
  }
  if (sem_post(shared_mem_->semaphores_.server_lock) == -1) { // release lock
    perror("sem_t SERVER_LOCK post failed");
    close_server(shared_mem_, shmid);
  }

  while (clients_left > 0) {
    // WAIT FOR A DELIVERY
    VLOG(DEBUG, "HERE");
    if (sem_wait(shared_mem_->semaphores_.server_queue) == -1) {
      perror("sem_t SERVER_QUEUE wait failed");
      close_server(shared_mem_, shmid);
    }
    VLOG(DEBUG, "HERE");
    int server_time = randomize_n(TIME_SERVER);
    shared_mem_->server_time = server_time;
    printf("Server serving client in... %d s\n", server_time);
    sleep(server_time);
    // SIGNAL THE FOOD HAS BEEN SERVED
    if (sem_post(shared_mem_->semaphores_.client_server) == -1) {
      perror("sem_t SERVER_LOCK post failed");
      close_server(shared_mem_, shmid);
    }
    clients_left = shared_mem_->counters_.client;
    VLOG(DEBUG, "Clients Left: %d", clients_left);
  }

  // DETACH MEM AND CLOSE SEM
  sem_close(shared_mem_->semaphores_.server_queue);
  sem_close(shared_mem_->semaphores_.server_lock);
  sem_close(shared_mem_->semaphores_.client_server);

  detach_shared_mem(shared_mem_, shmid);

  return EXIT_SUCCESS;
}

void close_server(shared_mem *shared_mem_, int shmid) {

  sem_close(shared_mem_->semaphores_.server_queue);
  sem_close(shared_mem_->semaphores_.server_lock);
  sem_close(shared_mem_->semaphores_.client_server);

  detach_shared_mem(shared_mem_, shmid);

  exit(EXIT_FAILURE);
}
