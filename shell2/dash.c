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
  int status, j, i, read_fd = 0, in_cpy, out_cpy;
  int fd[2];
  pid_t child_pid;

  // clear system
  system("clear");

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
    i = 0;
    char _input[MAX_INPUT_SIZE];
    write(1, ">> ", 3);
    // get input
    fgets(_input, MAX_INPUT_SIZE, stdin);
    parse_input(_input, &argv, &argc, "|", 1);

    if (argc == 1) {
      // save file descriptors for overwrite issue
      in_cpy = dup(STDIN_FILENO);
      out_cpy = dup(STDOUT_FILENO);
      // execute single command
      execute_command(_input, history, &hist_capacity, ONLY_CMD);
      // reset file descriptors
      dup2(in_cpy, STDIN_FILENO);
      dup2(out_cpy, STDOUT_FILENO);
      continue;
    }

    while (i < argc) {
      pipe(fd);
      switch ((child_pid = fork())) {
      case CHILD:
        if (i == FIRST_CMD) {
          dup2(fd[WRITE], WRITE);
          close(fd[READ]);
          execute_command(argv[i], history, &hist_capacity, FIRST_CMD);
          exit(EXIT_SUCCESS);
        } else if (i < argc - 1) { // NOT LAST COMMAND
          dup2(read_fd, READ);
          dup2(fd[WRITE], WRITE);
          close(fd[READ]);
          execute_command(argv[i], history, &hist_capacity, MIDDLE_CMD);
          exit(EXIT_SUCCESS);
        } else { // i is LAST COMMAND
          dup2(read_fd, READ);
          close(fd[READ]);
          execute_command(argv[i], history, &hist_capacity, LAST_CMD);
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
  }
  close(fd[READ]);
  close(fd[WRITE]);
  return EXIT_SUCCESS;
}
