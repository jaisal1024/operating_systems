#include "common.h"
#include "func.h"
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv) {
  // declare variables
  int hist_capacity = 0;
  int status, j;
  // get and store current directory
  if (init_dir() == FAILURE) {
    perror("initialize PATH failed");
    exit(EXIT_FAILURE);
  }
  // allocate history data structure
  char **history = malloc(MAX_HIST_SIZE * sizeof(char *));
  for (j = 0; j < MAX_HIST_SIZE; j++) {
    history[j] = malloc(MAX_INPUT_SIZE * sizeof(char));
  }
  // load history
  status = load_history(history, &hist_capacity);
  if (status == -1) { // if .history file does not exist notify user
    printf("Load History: Empty\n");
  }

  while (1) {
    write(1, ">> ", 3);
    execute_command(argc, argv, history, &hist_capacity);
  }
  return EXIT_SUCCESS;
}
