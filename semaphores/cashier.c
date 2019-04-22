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

int main(int argc, char **argv) {
  // INIT VARS
  int service_time, break_time, shmid, i, parameters = 0;
  // shared_mem *mem_ptr;
  shared_mem *shared_mem_;

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

  shared_mem_ = (shared_mem *)shmat(shmid, NULL, 0);
  if (*(int *)shared_mem_ == -1) {
    perror("Attach Shared Memory Failed");
    exit(EXIT_FAILURE);
  }
  // shared_mem_ = mem_ptr;
  printf("Shared memory attached at: %d\n", shmid);
  VLOG(DEBUG, "Menu 1: %d", shared_mem_->menu[0].price);
  if (sem_post(shared_mem_->sem_cashiers) == -1)
    perror("sem_post failed");

  return EXIT_SUCCESS;
}
