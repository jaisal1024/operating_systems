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

const char *SEM_CASHIER = "/SEM_CASHIER";
const char *SEM_CLIENT = "/SEM_CLIENT";
const char *SEM_LOCK = "/SEM_LOCK";

extern shared_mem *attach_shared_mem_and_open_sem(int shmid) {
  shared_mem *shared_mem_;
  shared_mem_ = (shared_mem *)shmat(shmid, NULL, 0);
  if ((int)shared_mem_ == -1) {
    perror("Attach Shared Memory Failed");
    exit(EXIT_FAILURE);
  }

  printf("Shared memory attached at: %d\n", shmid);

  shared_mem_->sem_cashiers = sem_open(SEM_CASHIER, 0);
  shared_mem_->sem_clients = sem_open(SEM_CLIENT, 0);
  shared_mem_->sem_binary_locking = sem_open(SEM_LOCK, 0);

  return shared_mem_;
}

extern void detach_shared_mem_and_close_sem(shared_mem *shared_mem_,
                                            int shmid) {

  sem_close(shared_mem_->sem_cashiers);
  sem_close(shared_mem_->sem_clients);
  sem_close(shared_mem_->sem_binary_locking);

  if (shmdt((void *)shared_mem_) == -1)
    perror("Shared Segment Detachment");
  printf("Shared memory detached at: %d\n", shmid);
}
