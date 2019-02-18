#include "common.h"
#include "func.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <unistd.h>

static void strip(char *, int, char **);

static void strip(char *_input, int argc, char **argv) {
  // replace special characters in input
  int i;
  for (i = 0; i < strlen(_input); i++) {
    if (_input[i] < 32) {
      _input[i] = '\0';
    }
  }
  // set to empty
  memset(argv, 0, argc * sizeof(argv));
}

void parse_input(char _input[], char **argv, int *argc) {
  // declerations
  const char *delim = " ";
  char *token;

  /* -------------parsed_input----------------------
     [0] = command
     [1-n] = flags and other identifiers
   */
  // strip input
  // strip(_input, parsed_input);
  // loop over input
  while ((token = strsep(&_input, delim)) != NULL) {
    if (*token != '\0') {
      if (token[strlen(token) - 1] < 32) {
        token[strlen(token) - 1] = '\0';
      }
      argv[(*argc)++] = token;
      // VLOG(DEBUG, "T:%s", token);
      // VLOG(DEBUG, "C:%i", *argc);
    }
  }
}

int load_history(char **history, int *hist_capacity) {
  FILE *fp;
  char *ln = NULL;
  size_t cap = MAX_INPUT_SIZE;
  ssize_t ln_size;
  fp = fopen(".history", "r");
  if (fp == NULL)
    return -1;
  while ((ln_size = getline(&ln, &cap, fp)) > 0) {
    snprintf(history[(*hist_capacity)++], ln_size, "%s", ln);
  }
  fclose(fp);
  return 0;
}

int update_history(char **history, char _input[], int *hist_capacity) {
  FILE *fp;
  if ((*hist_capacity) >= MAX_HIST_SIZE - 1) { // need to shift history array
    memmove(history, history + 1, (MAX_HIST_SIZE - 1) * sizeof(history[0]));
    history[MAX_HIST_SIZE - 1] = malloc(sizeof(char *));
    snprintf(history[MAX_HIST_SIZE - 1], MAX_INPUT_SIZE * sizeof(char), "%s",
             _input);
    fp = fopen(".history", "w");
    if (fp == NULL)
      return -1;
    int j;
    for (j = 0; j < MAX_HIST_SIZE; j++) {
      fprintf(fp, "%s\n", history[j]);
    }
  } else {
    strcpy(history[(*hist_capacity)++], _input);
    fp = fopen(".history", "a");
    if (fp == NULL)
      return -1;
    fprintf(fp, "%s\n", _input);
  }
  fclose(fp);
  return 0;
}

void print_path(path path_[], int path_capacity) {
  int i;
  for (i = 0; i < path_capacity; i++) {
    printf("%s=%s\n", path_[i].identifier, path_[i].body);
  }
}

int update_path(path *path_, char **argv, int *path_capacity) {
  path temp_path_;
  const char *delim = "=";
  char *token;
  int i = 0;
  int exists = 0;

  while ((token = strsep(&argv[IDENTIFIER_INDEX], delim)) != NULL) {
    if (*token != '\0') {
      if (!i++)
        strcpy(temp_path_.identifier, token);
      else
        strcpy(temp_path_.body, token);
      VLOG(DEBUG, "T:%s", token);
    }
  }
  int temp_len = strlen(temp_path_.identifier);
  for (i = 0; i < (*path_capacity); i++) {
    if (strlen(path_[i].identifier) == temp_len) {
      if (strncmp(temp_path_.identifier, path_[i].identifier, temp_len) == 0) {
        exists = 1;
        strcpy(path_[i].body, temp_path_.body);
        // VLOG(DEBUG, "UP:%s=%s", path_[i].identifier, path_[i].body);
        break;
      }
    }
  }
  if (!exists) {
    strcpy(path_[(*path_capacity)].identifier, temp_path_.identifier);
    strcpy(path_[(*path_capacity)++].body, temp_path_.body);
  }
  return 0;
}

int external_command(path path_, char **argv, int argc, int path_capacity) {
  if (strlen(path_.body) == 0) {
    return -1;
  }
  const char *delim = ":";
  char *path_ptr = malloc(sizeof(char *));
  char *token;
  char *cwd;
  char path_list[MAX_NUM_OF_PATHS_2][MAX_PATH_SIZE];
  int path_list_ind = 0;
  int i = 0;
  int found = 0;
  strncpy(path_ptr, path_.body, sizeof(path_.body));
  VLOG(DEBUG, "%s", path_ptr);

  while ((token = strsep(&path_ptr, delim)) != NULL) {
    if (*token != '\0') {
      VLOG(DEBUG, "TT: %s", token);
      strncpy(path_list[path_list_ind++], token, strlen(token) * sizeof(char));
    }
  }

  for (i = 0; i < path_list_ind; i++) {
    cwd = strcat(path_list[i], "/\0");
    cwd = strcat(cwd, argv[COMMAND_INDEX]);
    if (access(cwd, 1) == 0) {
      found = 1;
      printf("%s is an external command (%s)\n", argv[COMMAND_INDEX], cwd);
      break;
    }
    memset(cwd, 0, strlen(cwd) * sizeof(char));
  }

  if (found) {
    if (argc > 1) {
      printf("command arguments:\n\t");
      for (i = 1; i < argc; i++) {
        printf("%s ", argv[i]);
      }
      printf("\n");
    }
  }

  free(path_ptr);
  if (found)
    return 0;
  return -1;
}
