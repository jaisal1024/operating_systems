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

static void close_client(shared_mem *, int);

int main(int argc, char **argv) {
  // INIT VARS
  int item_id, eat_time, shmid, i, parameters = 0, clients_left = 0,
                                   pid = (int)getpid();
  shared_mem *shared_mem_;
  time_t arr_time, dep_time;
  srand(1);

  // READ ARGV INPUTS
  for (i = 1; i < argc - 1; i++) {
    if (argv[i][0] == '-' && argv[i][1] == 'i') {
      item_id = atoi(argv[i + 1]);
      parameters++;
      // ERROR CHECK
      if (item_id < 0) {
        fprintf(stderr, "%s\n", "Incorrect Arguments Passed: Bad item ID");
        exit(EXIT_FAILURE);
      }
    } else if (argv[i][0] == '-' && argv[i][1] == 'e') {
      eat_time = atoi(argv[i + 1]);
      parameters++;
      // ERROR CHECK
      if (eat_time < 0) {
        fprintf(stderr, "%s\n", "Incorrect Arguments Passed: Bad Eat Time");
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
  shared_mem_->semaphores_.client_cashier = sem_open(SEM_CLIENT_CASHIER, 0);
  shared_mem_->semaphores_.client_queue = sem_open(SEM_CLIENT_QUEUE, 0);
  shared_mem_->semaphores_.client_lock = sem_open(SEM_CLIENT_LOCK, 0);
  shared_mem_->semaphores_.server_queue = sem_open(SEM_SERVER_QUEUE, 0);
  shared_mem_->semaphores_.client_server = sem_open(SEM_CLIENT_SERVER, 0);

  // CHECK-IN : DECREMENT CLIENT QUEUE COUNTER IF NOT FULL
  if (sem_wait(shared_mem_->semaphores_.client_lock) == -1) { // acquire lock
    perror("sem_t CLIENT_LOCK wait failed");
    close_client(shared_mem_, shmid);
  }
  if (shared_mem_->counters_.client_queue > 0) {
    shared_mem_->counters_.client_queue--;
    clients_left = shared_mem_->counters_.client;
  } else {
    close_client(shared_mem_, shmid);
  }
  if (sem_post(shared_mem_->semaphores_.client_lock) == -1) { // release lock
    perror("sem_t CLIENT_LOCK post failed");
    close_client(shared_mem_, shmid);
  }
  // start arrival time when client enters the queue

  time(&arr_time);
  // SIGNAL CLIENT QUEUE
  if (sem_post(shared_mem_->semaphores_.client_queue) == -1) {
    perror("sem_t CLIENT_QUEUE post failed");
    close_client(shared_mem_, shmid);
  }
  printf("Client %d has entered the queue\n", pid);

  // WAIT FOR CASHIER QUEUE
  if (sem_wait(shared_mem_->semaphores_.cashier_queue) == -1) { // acquire lock
    perror("sem_t CASHIER_QUEUE wait failed");
    close_client(shared_mem_, shmid);
  }
  // ADD TO THE CLIENT QUEUE
  shared_mem_->counters_.client_queue++;

  // ADD ORDER TO SHARED_MEM
  /* you can access the client counter variable without an explicit lock
   * because the cashier is already locking here
   */
  int cur_client = MAX_CLIENTS - shared_mem_->counters_.client;
  shared_mem_->clients[cur_client].item_id = item_id;
  shared_mem_->clients[cur_client].client_id = pid;
  shared_mem_->clients[cur_client].eat_time = eat_time;
  shared_mem_->clients[cur_client].bill = shared_mem_->menu[item_id].price;
  shared_mem_->clients[cur_client].arrival_time = (double)arr_time;

  // TELL CASHIER DONE ORDERING
  if (sem_post(shared_mem_->semaphores_.client_cashier) == -1) {
    perror("sem_t CLIENT_CASHIER post failed");
    close_client(shared_mem_, shmid);
  }

  // WAIT TO BE SERVED BY CASHIER
  int cashier_time = shared_mem_->clients[cur_client].cashier_time;
  printf("Cashier serving client %d (%d) in... %ds", pid, cur_client,
         cashier_time);
  sleep(cashier_time);

  // WAIT FOR FOOD TO BE PREPARED
  int food_time = randomize_bt(shared_mem_->menu[item_id].max_time,
                               shared_mem_->menu[item_id].min_time);
  shared_mem_->clients[cur_client].food_time = food_time;
  printf("Food will be ready in... %ds", food_time);
  sleep(food_time);

  // JOIN SERVER QUEUE
  if (sem_post(shared_mem_->semaphores_.server_queue) == -1) {
    perror("sem_t SERVER_QUEUE post failed");
    close_client(shared_mem_, shmid);
  }
  // WAIT FOR SERVER TO SERVE FOOD
  if (sem_wait(shared_mem_->semaphores_.client_server) == -1) {
    perror("sem_t CLIENT_SERVER wait failed");
    close_client(shared_mem_, shmid);
  }
  // SLEEP FOR EAT TIME THEN EXIT
  sleep(eat_time);

  // CHECK IF THE LAST CLIENT
  if (cur_client <= 1) {
    shared_mem_->semaphores_.manager_lock = sem_open(SEM_MANAGER_LOCK, 0);
    if (sem_post(shared_mem_->semaphores_.manager_lock) == -1) {
      perror("sem_t MANAGER_LOCK post failed");
      close_client(shared_mem_, shmid);
    }
  }
  time(&dep_time);
  shared_mem_->clients[cur_client].depart_time = (double)dep_time;

  // CLOSE AND DETACH MEMORY
  detach_shared_mem(shared_mem_, shmid);
  sem_close(shared_mem_->semaphores_.client_queue);
  sem_close(shared_mem_->semaphores_.client_cashier);
  sem_close(shared_mem_->semaphores_.server_queue);
  sem_close(shared_mem_->semaphores_.client_server);
  sem_close(shared_mem_->semaphores_.client_lock);
  printf("Goodbye client - %d (%d)\n", pid, cur_client);

  return EXIT_SUCCESS;
}

void close_client(shared_mem *shared_mem_, int shmid) {

  detach_shared_mem(shared_mem_, shmid);
  sem_close(shared_mem_->semaphores_.client_queue);
  sem_close(shared_mem_->semaphores_.client_cashier);
  sem_close(shared_mem_->semaphores_.server_queue);
  sem_close(shared_mem_->semaphores_.client_server);
  sem_close(shared_mem_->semaphores_.client_lock);

  exit(EXIT_FAILURE);
}
