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

const char *SEM_MANAGER_LOCK = "/MANAGER_LOCK";
const char *SEM_CASHIER_QUEUE = "/CASHIER_QUEUE";
const char *SEM_CASHIER_LOCK = "/CASHIER_LOCK";
const char *SEM_CLIENT_CASHIER = "/CLIENT_CASHIER";
const char *SEM_CLIENT_QUEUE = "/CLIENT_QUEUE";
const char *SEM_CLIENT_SERVER = "/CLIENT_SERVER";
const char *SEM_CLIENT_LOCK = "/CLIENT_LOCK";
const char *SEM_SERVER_QUEUE = "/SERVER_QUEUE";
const char *SEM_SERVER_LOCK = "/SERVER_LOCK";

const char *database_dir = "database.txt";

extern int randomize_n(int max) { return rand() % max + 1; }
extern int randomize_bt(int max, int min) { return rand() % (max - min) + min; }

extern shared_mem *attach_shared_mem(int shmid) {
  shared_mem *shared_mem_;
  shared_mem_ = (shared_mem *)shmat(shmid, NULL, 0);
  // if (*(int *)shared_mem_ == -1) {
  if ((int)shared_mem_ == -1) {
    perror("Attach Shared Memory Failed Bad Memory Reference");
    exit(EXIT_FAILURE);
  }
  VLOG(DEBUG, "HERE");
  printf("Shared memory attached at: %d\n", shmid);

  return shared_mem_;
}

extern void detach_shared_mem(shared_mem *shared_mem_, int shmid) {
  if (shmdt((void *)shared_mem_) == -1)
    perror("Shared Segment Detachment");
  printf("Shared memory detached at: %d\n", shmid);
}

extern void detach_shared_mem_and_close_all_sem(shared_mem *shared_mem_,
                                                int shmid,
                                                semaphores semaphores_) {

  sem_close(semaphores_.manager_lock);
  sem_close(semaphores_.cashier_queue);
  sem_close(semaphores_.cashier_lock);
  sem_close(semaphores_.client_cashier);
  sem_close(semaphores_.client_queue);
  sem_close(semaphores_.client_lock);
  sem_close(semaphores_.client_server);
  sem_close(semaphores_.server_queue);
  sem_close(semaphores_.server_lock);

  if (shmdt((void *)shared_mem_) == -1)
    perror("Shared Segment Detachment");
  printf("Shared memory detached at: %d\n", shmid);

  /* close and remove semaphores */
  sem_unlink(SEM_MANAGER_LOCK);
  sem_unlink(SEM_CASHIER_QUEUE);
  sem_unlink(SEM_CASHIER_LOCK);
  sem_unlink(SEM_CLIENT_CASHIER);
  sem_unlink(SEM_CLIENT_QUEUE);
  sem_unlink(SEM_CLIENT_LOCK);
  sem_unlink(SEM_CLIENT_SERVER);
  sem_unlink(SEM_SERVER_QUEUE);
  sem_unlink(SEM_SERVER_LOCK);

  /* destroy shared memory */
  if (shmctl(shmid, IPC_RMID, (struct shmid_ds *)0) < 0) {
    perror("semctl");
    exit(EXIT_FAILURE);
  }
  printf("Releasing shared segment: %d\n", shmid);
}
