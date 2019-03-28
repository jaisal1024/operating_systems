#include "common.h"
#include "func.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char **argv) {
  int fd[2];
  pid_t child_pid;
  int i = 0, status, read_fd = 0;
  char *cat_cmd[]={"/bin/cat", "test", NULL};
  char *grep_cmd[]={"/usr/bin/grep", "hello", NULL};
  char *grep[]={"/usr/bin/grep", "there", NULL};
	char **cmd[] = {cat_cmd,grep_cmd, grep, NULL};
  argc = 3;
  fflush(stdin);
  fflush(stdout);

  while (cmd[i] != NULL) {
    pipe(fd);
    switch ((child_pid = fork())) {
    case CHILD:
      if (i < argc-1) { // NOT LAST COMMAND
        dup2(read_fd, READ);
        dup2(fd[WRITE], WRITE);
        close(fd[READ]);
        execv(cmd[i][0], cmd[i]);
        perror("exec failed");
        exit(EXIT_FAILURE);
      } else { // i is LAST COMMAND
        dup2(read_fd, READ);
        close(fd[READ]);
        execv(cmd[i][0], cmd[i]);
        perror("exec failed ");
        exit(EXIT_FAILURE);
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
  exit(EXIT_SUCCESS);
}
