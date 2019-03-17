#include "common.h"
#include "func.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char **argv) {
  char _input[MAX_INPUT_SIZE];
  int fd_pipe[2];
  int i = 0, status;
  argc = 0;
  memset(argv, 0, argc * sizeof(*argv));
  char *cmd[] = {"/exe", "main", NULL};

  // get input
  printf(">> ");
  fgets(_input, MAX_INPUT_SIZE, stdin);

  // parse input and check output
  parse_input(_input, argv, &argc, "|");
  debug_array(argv, argc, "Check strsep pipe");
  int child_ids[argc - 1];

  pipe(fd_pipe);

  while (i < argc) {
    VLOG(DEBUG, "%d", i);
    switch ((child_ids[i] = fork())) {
      fflush(stdin);
      fflush(stdout);
    case CHILD:
      if (i == FIRST_CMD) { // FIRST COMMAND
        close(fd_pipe[READ]);
        dup2(fd_pipe[WRITE], STDIN_FILENO);
        write(fd_pipe[WRITE], _input, strlen(_input));
        execv(cmd[0], cmd);
        perror("exec failed ");
      } else if (i < argc - 1) { // i is not FIRST or LAST COMMAND
        dup2(fd_pipe[WRITE], STDIN_FILENO);
        dup2(fd_pipe[READ], READ);
        //  write(fd_pipe[WRITE], fd_pipe[READ]);
        execv(cmd[0], cmd);
        perror("exec failed");
      } else { // i is LAST COMMAND
        close(fd_pipe[WRITE]);
        dup2(fd_pipe[READ], READ);
        execv(cmd[0], cmd);
        perror("exec failed ");
        exit(EXIT_SUCCESS);
      }
      break;

    case FORK_FAILED:
      perror("fork failed ");
      exit(EXIT_FAILURE);
    default:
      waitpid(child_ids[i++], &status, 0);
      break;
    }
  }

  close(fd_pipe[READ]);
  close(fd_pipe[WRITE]);
  exit(EXIT_SUCCESS);
}
