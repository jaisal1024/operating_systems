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

  // // INIT SEMAPHORES
  // shared_mem_->semaphores_.manager_lock =
  //     sem_open(SEM_MANAGER_LOCK, O_CREAT, RW_ACCESS, 0);
  // if (shared_mem_->semaphores_.manager_lock == SEM_FAILED) {
  //   perror("Sem_t MANAGER_LOCK Open Failed");
  //   exit(EXIT_FAILURE);
  // }
  //
  // shared_mem_->semaphores_.cashier_queue =
  //     sem_open(SEM_CASHIER_QUEUE, O_CREAT, RW_ACCESS, 0);
  // if (shared_mem_->semaphores_.cashier_queue == SEM_FAILED) {
  //   perror("Sem_t CASHIER_QUEUE Open Failed");
  //   exit(EXIT_FAILURE);
  // }
  //
  // shared_mem_->semaphores_.cashier_lock =
  //     sem_open(SEM_CASHIER_LOCK, O_CREAT, RW_ACCESS, 1);
  // if (shared_mem_->semaphores_.cashier_lock == SEM_FAILED) {
  //   perror("Sem_t CASHIER_LOCK Open Failed");
  //   exit(EXIT_FAILURE);
  // }
  //
  // shared_mem_->semaphores_.client_cashier =
  //     sem_open(SEM_CLIENT_CASHIER, O_CREAT, RW_ACCESS, 0);
  // if (shared_mem_->semaphores_.client_cashier == SEM_FAILED) {
  //   perror("Sem_t CLIENT_CASHIER Open Failed");
  //   exit(EXIT_FAILURE);
  // }
  //
  // shared_mem_->semaphores_.client_queue =
  //     sem_open(SEM_CLIENT_QUEUE, O_CREAT, RW_ACCESS, 0);
  // if (shared_mem_->semaphores_.client_queue == SEM_FAILED) {
  //   perror("Sem_t CLIENT_QUEUE Open Failed");
  //   exit(EXIT_FAILURE);
  // }
  //
  // shared_mem_->semaphores_.client_lock =
  //     sem_open(SEM_CLIENT_LOCK, O_CREAT, RW_ACCESS, 1);
  // if (shared_mem_->semaphores_.client_lock == SEM_FAILED) {
  //   perror("Sem_t CLIENT LOCK Open Failed");
  //   exit(EXIT_FAILURE);
  // }
  //
  // shared_mem_->semaphores_.client_server =
  //     sem_open(SEM_CLIENT_SERVER, O_CREAT, RW_ACCESS, 0);
  // if (shared_mem_->semaphores_.client_server == SEM_FAILED) {
  //   perror("Sem_t CLIENT_SERVER Open Failed");
  //   exit(EXIT_FAILURE);
  // }
  //
  // shared_mem_->semaphores_.server_queue =
  //     sem_open(SEM_SERVER_QUEUE, O_CREAT, RW_ACCESS, 0);
  // if (shared_mem_->semaphores_.server_queue == SEM_FAILED) {
  //   perror("Sem_t SERVER_QUEUE Open Failed");
  //   exit(EXIT_FAILURE);
  // }
  //
  // shared_mem_->semaphores_.server_lock =
  //     sem_open(SEM_SERVER_LOCK, O_CREAT, RW_ACCESS, 1);
  // if (shared_mem_->semaphores_.server_lock == SEM_FAILED) {
  //   perror("Sem_t SERVER_LOCK Open Failed");
  //   exit(EXIT_FAILURE);
  // }

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
