#include "common.h"
#include "func.h"
#include <ctype.h>
#include <errno.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv) {
  // declare variables
  int exit = 0, hist_capacity = 0, path_capacity = 0;
  int j, status, i;
  char *wd = malloc((sizeof(char) * MAX_PATH_SIZE));
  // allocate history data structure
  char **history = malloc(MAX_HIST_SIZE * sizeof(char *));
  for (j = 0; j < MAX_HIST_SIZE; j++) {
    history[j] = malloc(MAX_INPUT_SIZE * sizeof(char));
  }
  status = load_history(history, &hist_capacity);
  if (status == -1) {
    printf("Load History Failed\n");
  }
  // declare PATH data structure
  path path_[MAX_NUM_OF_PATHS];
  strcpy(path_[PATH_INDEX].identifier, "PATH");
  strcpy(path_[PATH_INDEX].body, "");
  path_capacity++;

  while (!exit) {
    // declerations
    char _input[MAX_INPUT_SIZE];
    argc = 0;

    // get input
    printf(">> ");
    fgets(_input, MAX_INPUT_SIZE, stdin);

    // parse input and check output
    parse_input(_input, argv, &argc);
    if (argc > 0) {
      if (argv[COMMAND_INDEX][0] - '!' == 0) {
        if (isdigit(argv[COMMAND_INDEX][1])) {
          char *buf = malloc(2 * sizeof(char));
          strncpy(buf, argv[COMMAND_INDEX] + 1, 2);
          int ind = atoi(buf) - 1;
          if (ind <= hist_capacity) {
            argc = 0;
            memset(argv, 0, MAX_TOKEN_SIZE * sizeof(&argv));
            strcpy(_input, history[ind]);
            parse_input(_input, argv, &argc);
          }
        }
      }
      if (strncmp(argv[COMMAND_INDEX], "cd", 2) == 0) {
        if (argc < 2) {
          getcwd(wd, MAX_PATH_SIZE);
          char *cwd = dirname(wd);
          if (access(cwd, 1) == 0) {
            if (chdir(cwd) != -1)
              status = update_history(history, _input, &hist_capacity);
            else
              printf("%s\n", strerror(errno));
          } else {
            printf("%s\n", strerror(errno));
          }
        } else {
          getcwd(wd, MAX_PATH_SIZE);
          char *cwd = strcat(wd, "/\0");
          cwd = strcat(cwd, argv[IDENTIFIER_INDEX]);
          if (access(cwd, 1) == 0) {
            if (chdir(cwd) != -1)
              status = update_history(history, _input, &hist_capacity);
            else
              printf("%s\n", strerror(errno));
          } else {
            printf("%s\n", strerror(errno));
          }
        }
      } else if (strncmp(argv[COMMAND_INDEX], "pwd", 3) == 0) {
        getcwd(wd, MAX_PATH_SIZE);
        printf("%s\n", wd);
        status = update_history(history, _input, &hist_capacity);
      } else if (strncmp(argv[COMMAND_INDEX], "exit", 4) == 0) {
        VLOG(INFO, "Dash: bye");
        status = update_history(history, _input, &hist_capacity);
        exit = 1;
      } else if (strncmp(argv[COMMAND_INDEX], "export", 6) == 0 && argc <= 2) {
        if (argc < 2) {
          print_path(path_, path_capacity);
        } else {
          status = update_path(path_, argv, &path_capacity);
        }
        status = update_history(history, _input, &hist_capacity);
      } else if (strncmp(argv[COMMAND_INDEX], "history", 7) == 0) {
        status = update_history(history, _input, &hist_capacity);
        for (i = 0; i < hist_capacity; i++) {
          printf("\t%i %s\n", i + 1, history[i]);
        }
      } else {
        status = external_command(path_[PATH_INDEX], argv, argc, path_capacity);
        if (status == -1)
          printf("command not found\n");
        else {
          status = update_history(history, _input, &hist_capacity);
        }
      }
    } else {
      printf("command not found\n");
    }
  }

  for (j = 0; j < MAX_HIST_SIZE; j++) {
    free(history[j]);
  }
  free(history);
  free(wd);

  return 0;
}
