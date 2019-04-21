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

int main(int argc, char **argv) {
  // INIT VARS
  int i, shmid, parameters = 0;

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

  return EXIT_SUCCESS;
}
