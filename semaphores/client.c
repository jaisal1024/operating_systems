#include "common.h"
#include "func.h"
#include <ctype.h>
#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <time.h>
#include <unistd.h>

static void close_client(shared_mem *, int, semaphores);

int main(int argc, char **argv) {
  // INIT VARS
  int item_id, eat_time, shmid, i, parameters = 0, pid = (int)getpid(),
                                   clients_left = 0;
  shared_mem *shared_mem_;
  semaphores semaphores_;
  time_t arr_time, dep_time;
  srand(1);

  // READ ARGV INPUTS
  for (i = 1; i < argc - 1; i++) {
    if (argv[i][0] == '-' && argv[i][1] == 'i') {
      item_id = atoi(argv[i + 1]);
      item_id--;
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
  semaphores_.client_cashier = sem_open(SEM_CLIENT_CASHIER, 0);
  semaphores_.cashier_queue = sem_open(SEM_CASHIER_QUEUE, 0);
  semaphores_.client_queue = sem_open(SEM_CLIENT_QUEUE, 0);
  semaphores_.client_lock = sem_open(SEM_CLIENT_LOCK, 0);
  semaphores_.server_queue = sem_open(SEM_SERVER_QUEUE, 0);
  semaphores_.client_server = sem_open(SEM_CLIENT_SERVER, 0);

  // CHECK-IN : DECREMENT CLIENT QUEUE COUNTER IF NOT FULL
  if (sem_wait(semaphores_.client_lock) == -1) { // acquire lock
    perror("sem_t CLIENT_LOCK wait failed");
    close_client(shared_mem_, shmid, semaphores_);
  }
  if (shared_mem_->counters_.client_queue > 0) {
    shared_mem_->counters_.client_queue--;
    clients_left = shared_mem_->counters_.client;
  } else {
    if (sem_post(semaphores_.client_lock) == -1) { // release lock
      perror("sem_t CLIENT_LOCK post failed");
      close_client(shared_mem_, shmid, semaphores_);
    }
    fprintf(stderr, "The Restaurant Queue is too long\n");
    close_client(shared_mem_, shmid, semaphores_);
  }
  if (sem_post(semaphores_.client_lock) == -1) { // release lock
    perror("sem_t CLIENT_LOCK post failed");
    close_client(shared_mem_, shmid, semaphores_);
  }
  // start arrival time when client enters the queue
  time(&arr_time);

  // SIGNAL CLIENT QUEUE
  if (sem_post(semaphores_.client_queue) == -1) {
    perror("sem_t CLIENT_QUEUE post failed");
    close_client(shared_mem_, shmid, semaphores_);
  }
  printf("Client %d has entered the cashier queue at %s", pid,
         ctime(&arr_time));

  // WAIT FOR CASHIER QUEUE
  VLOG(DEBUG, "WAITING IN CASHIER QUEUE");
  if (sem_wait(semaphores_.cashier_queue) == -1) {
    perror("sem_t CASHIER_QUEUE wait failed");
    close_client(shared_mem_, shmid, semaphores_);
  }
  // ADD TO THE CLIENT QUEUE
  shared_mem_->counters_.client_queue++;

  // ADD ORDER TO SHARED_MEM
  /* you can access the client counter variable without an explicit lock
   * because the cashier is already locking here
   */
  int cur_client = MAX_CLIENTS - shared_mem_->counters_.client;
  VLOG(DEBUG, "Cur_client: %d", cur_client);
  shared_mem_->clients[cur_client].item_id = item_id;
  shared_mem_->clients[cur_client].client_id = pid;
  shared_mem_->clients[cur_client].eat_time = eat_time;
  shared_mem_->clients[cur_client].bill = shared_mem_->menu[item_id].price;
  shared_mem_->clients[cur_client].arrival_time = (double)arr_time;

  // TELL CASHIER DONE ORDERING
  if (sem_post(semaphores_.client_cashier) == -1) {
    perror("sem_t CLIENT_CASHIER post failed");
    close_client(shared_mem_, shmid, semaphores_);
  }
  VLOG(DEBUG, "DONE ORDERING, CLIENTS LEFT: %d", shared_mem_->counters_.client);

  // WAIT TO BE SERVED BY CASHIER
  int cashier_time = shared_mem_->clients[cur_client].cashier_time;
  printf("Cashier serving client %d (%d) in... %ds\n", cur_client, pid,
         cashier_time);
  sleep(cashier_time);

  // WAIT FOR FOOD TO BE PREPARED
  int food_time = randomize_bt(shared_mem_->menu[item_id].max_time,
                               shared_mem_->menu[item_id].min_time);
  shared_mem_->clients[cur_client].food_time = food_time;
  printf("Food will be ready in... %ds\n", food_time);
  sleep(food_time);

  // JOIN SERVER QUEUE
  if (sem_post(semaphores_.server_queue) == -1) {
    perror("sem_t SERVER_QUEUE post failed");
    close_client(shared_mem_, shmid, semaphores_);
  }
  printf("Client %d (%d) has entered the serving queue\n", cur_client, pid);
  // WAIT FOR SERVER TO SERVE FOOD
  if (sem_wait(semaphores_.client_server) == -1) {
    perror("sem_t CLIENT_SERVER wait failed");
    close_client(shared_mem_, shmid, semaphores_);
  }
  shared_mem_->clients[cur_client].server_time = shared_mem_->server_time;
  shared_mem_->server_time = 0;
  // SLEEP FOR EAT TIME THEN EXIT
  printf("Client %d (%d) is eating for %ds\n", cur_client, pid, eat_time);
  sleep(eat_time);

  // CHECK IF THE LAST CLIENT
  if ((MAX_CLIENTS - cur_client - 1) < 1) {
    semaphores_.manager_lock = sem_open(SEM_MANAGER_LOCK, 0);
    if (sem_post(semaphores_.manager_lock) == -1) {
      perror("sem_t MANAGER_LOCK post failed");
      close_client(shared_mem_, shmid, semaphores_);
    }
  }
  time(&dep_time);
  shared_mem_->clients[cur_client].depart_time = (double)dep_time;

  // SAY GOODBYE
  printf("Goodbye Client %d (%d) at %s", cur_client, pid, ctime(&dep_time));

  // CLOSE AND DETACH MEMORY
  sem_close(semaphores_.client_queue);
  sem_close(semaphores_.client_cashier);
  sem_close(semaphores_.cashier_queue);
  sem_close(semaphores_.server_queue);
  sem_close(semaphores_.client_server);
  sem_close(semaphores_.client_lock);

  detach_shared_mem(shared_mem_, shmid);

  return EXIT_SUCCESS;
}

void close_client(shared_mem *shared_mem_, int shmid, semaphores semaphores_) {

  sem_close(semaphores_.client_queue);
  sem_close(semaphores_.client_cashier);
  sem_close(semaphores_.cashier_queue);
  sem_close(semaphores_.server_queue);
  sem_close(semaphores_.client_server);
  sem_close(semaphores_.client_lock);

  detach_shared_mem(shared_mem_, shmid);

  exit(EXIT_FAILURE);
}
