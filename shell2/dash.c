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
  int status, j, i = 0, read_fd = 0;
  int fd[2];
  pid_t child_pid;
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
    char _input[MAX_INPUT_SIZE];
    write(1, ">> ", 3);
    // get input
    fgets(_input, MAX_INPUT_SIZE, stdin);
    parse_input(_input, &argv, &argc, "|", 1);

    if (argc == 1){
      execute_command(_input, history, &hist_capacity);
      continue;
    }
    fflush(stdin);
    fflush(stdout);

    while (i < argc) {
      pipe(fd);
      switch ((child_pid = fork())) {
      case CHILD:
        if (i < argc-1) { // NOT LAST COMMAND
          dup2(read_fd, READ);
          dup2(fd[WRITE], WRITE);
          close(fd[READ]);
          execute_command(argv[i], history, &hist_capacity);
          exit(EXIT_SUCCESS);
        } else { // i is LAST COMMAND
          dup2(read_fd, READ);
          close(fd[READ]);
          execute_command(argv[i], history, &hist_capacity);
          exit(EXIT_SUCCESS);
        }
        break;

      case FORK_FAILED:
        perror("fork failed");
        exit(EXIT_FAILURE);

      default:
        waitpid(child_pid, &status, 0);
        close(fd[WRITE]);
  			read_fd = fd[READ];
        break;
      }
      i++;
    }

    close(fd[READ]);
    close(fd[WRITE]);
  }
  return EXIT_SUCCESS;
}
