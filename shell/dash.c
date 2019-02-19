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
  int exit_ = 0, hist_capacity = 0, path_capacity = 0;
  int j, status, i;
  // get and store current directory
  char *cur_dir = malloc((sizeof(char) * MAX_PATH_SIZE));
  cur_dir = getcwd(cur_dir, MAX_PATH_SIZE);
  const char *home_dir = strdup(cur_dir);
  set_dir(home_dir);
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
  // declare PATH data structure
  path path_[MAX_NUM_OF_PATHS];
  strcpy(path_[PATH_INDEX].identifier, "PATH");
  strcpy(path_[PATH_INDEX].body, "");
  path_capacity++;

  while (!exit_) {
    // declerations
    char _input[MAX_INPUT_SIZE];
    argc = 0;

    // get input
    printf(">> ");
    fgets(_input, MAX_INPUT_SIZE, stdin);

    // parse input and check output
    parse_input(_input, argv, &argc);
    debug_array(argv, argc, "TOKEN");
    if (argc > 0) { // check input is tokenized as >1
      // check reference to history
      if (argv[COMMAND_INDEX][0] - '!' == 0) {
        if (isdigit(argv[COMMAND_INDEX][1])) {
          // convert 2nd and 3rd element of char array to number for hist
          char *buf = malloc(2 * sizeof(char));
          strncpy(buf, argv[COMMAND_INDEX] + 1, 2);
          int ind = atoi(buf) - 1;
          free(buf);
          if (ind <= hist_capacity) {
            // re-parse the input from the historical input line
            argc = 0;
            memset(argv, 0, MAX_TOKEN_SIZE * sizeof(&argv));
            strcpy(_input, history[ind]);
            parse_input(_input, argv, &argc);
          }
        }
      } else {
        // if not a reference to memory, add to the history
        status = update_history(history, _input, &hist_capacity);
      }

      if (strncmp(argv[COMMAND_INDEX], "cd", 2) == 0) {
        // if just "cd" is passed, go to home directory
        if (argc < 2) {
          // update cur_dir to home directory
          VLOG(DEBUG, "CD: %s", home_dir);
          if (access(home_dir, 1) == 0) {
            if (chdir(home_dir) == 0)
              getcwd(cur_dir, MAX_PATH_SIZE);
            else
              printf("%s\n", strerror(errno));
          } else {
            printf("%s\n", strerror(errno));
          }
        } else {
          if (strncmp(argv[IDENTIFIER_INDEX], "..", 2) == 0) {
            // update cur_dir to parent directory
            char *copy_dir = strcat(cur_dir, "/\0");
            copy_dir = dirname(copy_dir);
            if (access(copy_dir, 1) == 0) {
              if (chdir(copy_dir) == 0)
                getcwd(cur_dir, MAX_PATH_SIZE);
              else
                printf("%s\n", strerror(errno));
            } else {
              printf("%s\n", strerror(errno));
            }
          } else {
            char *copy_dir = strcat(cur_dir, "/\0");
            copy_dir = strcat(copy_dir, argv[IDENTIFIER_INDEX]);
            if (access(copy_dir, 1) == 0) {
              if (chdir(copy_dir) ==
                  0) // if successful, reset the current directory
                getcwd(cur_dir, MAX_PATH_SIZE);
              else
                printf("%s\n", strerror(errno));
            } else {
              printf("%s\n", strerror(errno));
            }
          }
        }

      } else if (strncmp(argv[COMMAND_INDEX], "pwd", 3) == 0) {
        printf("%s\n", cur_dir);

      } else if (strncmp(argv[COMMAND_INDEX], "exit", 4) == 0) {
        // free allocated memory and set exit to true
        write_to_history(history, hist_capacity);
        for (j = 0; j < MAX_HIST_SIZE; j++) {
          free(history[j]);
        }
        free(history);
        free(cur_dir);
        free((char *)home_dir);
        exit_ = 1;
        exit(EXIT_SUCCESS);

      } else if (strncmp(argv[COMMAND_INDEX], "export", 6) == 0 && argc <= 2) {
        if (argc < 2) { // print path_ export object if argc = 1
          print_path(path_, path_capacity);
        } else { // call update_path if argc = 2
          status = update_path(path_, argv, &path_capacity);
        }

      } else if (strncmp(argv[COMMAND_INDEX], "history", 7) == 0) {
        // print history object
        for (i = 0; i < hist_capacity; i++) {
          // check if new line is needed to add to printf statement
          if (history[i][strlen(history[i])] == '\n')
            printf("\t%i %s", i + 1, history[i]);
          else
            printf("\t%i %s\n", i + 1, history[i]);
        }

      } else {
        // check for external command using path_[PATH_INDEX] variable
        debug_array(argv, argc, "NF");
        status = external_command(path_[PATH_INDEX], argv, argc, path_capacity);
        if (status == -1)
          printf("command not found\n");
      }

    } // argc is 0, nothing passed
  }
  return EXIT_SUCCESS;
}
