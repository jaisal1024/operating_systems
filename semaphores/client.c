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
  int item_id, eat_time, shmid, i, parameters = 0;

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

  return EXIT_SUCCESS;
}
