#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "func.h"
#include "common.h"
#include <libgen.h>
#include <errno.h>

int main(int argc, char **argv) {
  int exit = 0;
  char* wd;
  while (!exit) {
    //declerations
    char _input[MAX_INPUT_SIZE];
    int i;
    argc = 0;

    //get input
    printf(">> ");
    fgets(_input, MAX_INPUT_SIZE, stdin);

    //parse input and check output
    parse_input(_input, argv, &argc);
    if (argc>0) {
        if (strncmp(argv[COMMAND_INDEX], "exit", 4)==0){
          VLOG(INFO, "Dash: bye");
          exit = 1;
        } else if(strncmp(argv[COMMAND_INDEX], "pwd", 3)==0){
          getwd(wd);
          printf("%s\n", wd);
        } else if (strncmp(argv[COMMAND_INDEX], "cd", 2)==0) {
          if (argc < 2){
            getwd(wd);
            char* cwd = dirname(wd);
            if (access(cwd, 1)==0){
              if (chdir(cwd)== -1){
                printf("%s\n", strerror(errno));
              }
            } else {
              printf("%s\n", strerror(errno));
            }
          } else {
            getwd(wd);
            char* cwd = strcat(wd, "/\0");
            cwd = strcat(cwd, argv[IDENTIFIER_INDEX]);
            if (access(cwd, 1)==0) {
              if (chdir(cwd) == -1) {
                printf("%s\n", strerror(errno));
              }
            } else {
              printf("%s\n", strerror(errno));
              // VLOG(DEBUG, "STDERR: %d-%s", errno, strerror(errno));
            }
          }
        } else {
          printf("command not found\n");
        }
    } else {
      printf("command not found\n");
    }
  }
  return 0;
}
