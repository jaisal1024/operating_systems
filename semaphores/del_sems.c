#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <unistd.h>

#define RW_ACCESS 0666

int main(int argc, char const *argv[]) {
  const char *SEM_MANAGER_LOCK = "/MANAGER_LOCK";
  const char *SEM_CASHIER_QUEUE = "/CASHIER_QUEUE";
  const char *SEM_CASHIER_LOCK = "/CASHIER_LOCK";
  const char *SEM_CLIENT_CASHIER = "/CLIENT_CASHIER";
  const char *SEM_CLIENT_QUEUE = "/CLIENT_QUEUE";
  const char *SEM_CLIENT_SERVER = "/CLIENT_SERVER";
  const char *SEM_CLIENT_LOCK = "/CLIENT_LOCK";
  const char *SEM_SERVER_QUEUE = "/SERVER_QUEUE";
  const char *SEM_SERVER_LOCK = "/SERVER_LOCK";

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

  printf("Destroyed all Semaphores links\n");

  return EXIT_SUCCESS;
}
