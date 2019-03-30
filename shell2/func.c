#include "common.h"
#include "func.h"
#include <ctype.h>
#include <fcntl.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static int replace_path_var_and_indirect(int *, char ***, int);
static int redirect_input(char *);
static int redirect_output(char *);
static void shift_array(char ***, int *, int);

static char *history_file_dir;
extern char **environ;

static int replace_path_var_and_indirect(int *_argc, char ***_argv, int cmd) {
  char **argv = *_argv;
  int argc = *_argc;
  int ret = SUCCESS;
  int i, loc;
  for (i = 1; i < argc; i++) {
    // redirect input
    if ((cmd == FIRST_CMD || cmd == ONLY_CMD) && argv[i][0] == '<' &&
        (i + 1) < argc) {
      ret = redirect_input(argv[i + 1]);
      shift_array(&argv, &argc, i);
      i -= 2;
      // redirect output
    } else if ((cmd == LAST_CMD || cmd == ONLY_CMD) && argv[i][0] == '>' &&
               (i + 1) < argc) {
      ret = redirect_output(argv[i + 1]);
      shift_array(&argv, &argc, i);
      i -= 2;
    } else {
      // check to replace $
      loc = strcspn(argv[i], "$");
      if (loc == strlen(argv[i]))
        continue;
      char *cpy = malloc(MAX_PATH_ID_SIZE);
      char *dup = strdup(argv[i] + loc + 1);
      memset(cpy, 0, MAX_PATH_ID_SIZE);
      memcpy(cpy, dup, strlen(dup));
      if (getenv(cpy) == NULL) {
        free(cpy);
        free(dup);
        continue;
      }
      char *env = strndup(getenv(cpy), strlen(getenv(cpy)) + 1);
      argv[i] = realloc(argv[i], strlen(argv[i]) + strlen(env) + 1);
      memmove(argv[i] + loc, env, strlen(env) + 1);
      free(dup);
      free(env);
      free(cpy);
    }
  }
  return ret;
}

static void shift_array(char ***_argv, int *_argc, int start) {
  // shift argv array 2 places left for indirection, effectively removing the
  // indirection commands
  char **argv = *_argv;
  int argc = *_argc;
  int i;

  for (i = start; i < argc; i++) {
    argv[i] = argv[i + 2];
  }
  if (start == argc - 2) {
    argv[start] = NULL;
    argv[start + 1] = NULL;
  } else {
    argv[argc - 1] = NULL;
    argv[argc - 2] = NULL;
  }
  *_argc = argc - 2;
}

void execute_command(char *_input, char **history, int *hist_capacity,
                     int cmd) {
  // declerations
  int j, status, i;
  int _argc = 0;
  char **_argv = malloc(MAX_TOKEN_SIZE * sizeof(char *));
  for (j = 0; j < MAX_TOKEN_SIZE; j++) {
    _argv[j] = malloc(MAX_INPUT_SIZE * sizeof(char));
    if (_argv[j] == NULL) {
      perror("memory allocation failed");
      exit(EXIT_FAILURE);
    }
  }

  // check for historical reference
  strip(_input);
  hist_reference(_input, history, *hist_capacity);

  // parse input and check output
  parse_input(_input, &_argv, &_argc, " ", 0);
  status = replace_path_var_and_indirect(&_argc, &_argv, cmd);

  if (_argc > 0 && status != FAILURE) { // check input is tokenized as >1
                                        // check not failed on indirect
    if (strncmp(_argv[COMMAND_INDEX], "cd", 2) == 0) {
      // if just "cd" is passed, go to home directory
      if (_argc < 2) {
        // update PWD to home directory
        char *home_dir = getenv("HOME");
        if (access(home_dir, 1) == 0) {
          if (chdir(home_dir) == 0)
            setenv("PWD", home_dir, 1);
          else
            perror(NULL);
        } else {
          perror(NULL);
        }
      } else {
        if (strncmp(_argv[IDENTIFIER_INDEX], "..", 2) == 0) {
          // update PWD to parent directory
          char *copy_dir = dirname(getenv("PWD"));
          if (access(copy_dir, 1) == 0) {
            if (chdir(copy_dir) == 0)
              setenv("PWD", copy_dir, 1);
            else
              perror(NULL);
          } else {
            perror(NULL);
          }
        } else {
          char *copy_dir = malloc(MAX_PATH_SIZE);
          snprintf(copy_dir, strlen(getenv("PWD")) + 1, "%s", getenv("PWD"));
          copy_dir = strncat(copy_dir, "/\0", 2);
          copy_dir = strncat(copy_dir, _argv[IDENTIFIER_INDEX],
                             strlen(_argv[IDENTIFIER_INDEX]) + 1);
          if (access(copy_dir, 1) == 0) {
            if (chdir(copy_dir) ==
                0) // if successful, reset the current directory
              setenv("PWD", copy_dir, 1);
            else
              perror(NULL);
          } else {
            perror(NULL);
          }
          free(copy_dir);
        }
      }

    } else if (strncmp(_argv[COMMAND_INDEX], "pwd", 3) == 0) {
      printf("%s\n", getenv("PWD"));

    } else if (strncmp(_argv[COMMAND_INDEX], "exit", 4) == 0) {
      // free allocated memory and set exit to true
      write_to_history(history, *hist_capacity);
      for (j = 0; j < MAX_HIST_SIZE; j++) {
        free(history[j]);
      }
      for (j = 0; j < MAX_TOKEN_SIZE; j++) {
        free(_argv[j]);
      }
      free(history);
      free(_argv);
      free(history_file_dir);
      exit(EXIT_SUCCESS);

    } else if (strncmp(_argv[COMMAND_INDEX], "export", 6) == 0 && _argc <= 2) {
      if (_argc < 2) { // print path_ export object if argc = 1
        print_path();
      } else { // call update_path if argc = 2
        status = update_path(_argv);
      }

    } else if (strncmp(_argv[COMMAND_INDEX], "history", 7) == 0) {
      // print history object
      for (i = 0; i < *hist_capacity; i++) {
        // check if new line is needed to add to printf statement
        if (history[i][strlen(history[i])] == '\n')
          printf("\t%i %s", i + 1, history[i]);
        else
          printf("\t%i %s\n", i + 1, history[i]);
      }

    } else {
      // check for external command using path_[PATH_INDEX] variable
      status = external_command(_argv, _argc, cmd);
      if (status == FAILURE)
        fprintf(stderr, "command not found\n");
    }
  } // argc is 0, nothing passed or failed to redirect input/output
}

int hist_reference(char *_input, char **history, int hist_capacity) {
  // replace input with historical reference
  if (strlen(_input) < 4 && _input[0] == '!') {
    if (isdigit(_input[1])) {
      // convert 2nd and 3rd element of char array to number for hist
      char *buf = malloc(2 * sizeof(char));
      strncpy(buf, _input + 1, 2);
      int ind = atoi(buf) - 1;
      free(buf);
      if (ind <= hist_capacity) {
        // re-parse the input from the historical input line
        snprintf(_input, strlen(history[ind]) + 1, "%s", history[ind]);
        return FOUND;
      }
    }
  }
  return NOT_FOUND;
}

void strip(char *_input) {
  // replace special characters in input
  int i;
  for (i = 0; i < strlen(_input) + 1; i++) {
    if (_input[i] < 32) {
      _input[i] = '\0';
    }
  }
}

// set the home directory for the history file to access
int init_dir() {
  char *cur_dir = malloc((sizeof(char) * MAX_PATH_SIZE));
  cur_dir = getcwd(cur_dir, MAX_PATH_SIZE);
  if (cur_dir == NULL)
    return FAILURE;
  int status = setenv("PWD", cur_dir, 1) & setenv("HOME", cur_dir, 1) &
               setenv("PATH", "", 1);

  // set the home dir as the original path upon invokation
  history_file_dir = malloc(strlen(cur_dir) * sizeof(char) + sizeof(char) * 10);
  history_file_dir = strncpy(history_file_dir, cur_dir, strlen(cur_dir));
  history_file_dir = strncat(history_file_dir, "/.history", sizeof(char) * 10);
  return status && history_file_dir != NULL;
}

void parse_input(char _input[], char ***_argv, int *argc, const char *delim,
                 int main) {
  char **argv = *_argv;
  // declerations
  char *token;
  *argc = 0;
  // argv[*(argc)++] = "./dash\0";
  char *input_cpy = strdup(_input);

  /* -------------parsed_input----------------------
     [0] = command
     [1-n] = flags and other identifiers
   */
  // strip input
  // strip(_input, parsed_input);
  // loop over input
  while ((token = strsep(&input_cpy, delim)) != NULL) {
    if (*token != '\0') {
      if (token[strlen(token) - 1] < 32) {
        token[strlen(token) - 1] = '\0';
      }
      if (main)
        argv[(*argc)++] = token;
      else
        snprintf(argv[(*argc)++], strlen(token) + 1, "%s", token);
    }
  }
  free(input_cpy);
}

// load history from .history file in home directory
int load_history(char **history, int *hist_capacity) {
  FILE *fp;
  char *ln = NULL;
  size_t cap = MAX_INPUT_SIZE;
  ssize_t ln_size;
  fp = fopen(history_file_dir, "r");
  if (fp == NULL)
    return -1;
  while ((ln_size = getline(&ln, &cap, fp)) > 0 &&
         (*hist_capacity) < MAX_HIST_SIZE) {
    snprintf(history[(*hist_capacity)++], ln_size, "%s", ln);
  }
  fclose(fp);
  return 0;
}

int update_history(char **history, char _input[], int *hist_capacity) {
  if ((*hist_capacity) >= MAX_HIST_SIZE - 1) {
    // shift memory to delete the first element of history
    memmove(history, history + 1, (MAX_HIST_SIZE - 1) * sizeof(history[0]));
    history[MAX_HIST_SIZE - 1] = malloc(sizeof(char *));
    snprintf(history[MAX_HIST_SIZE - 1], MAX_INPUT_SIZE * sizeof(char), "%s",
             _input);
    // FOR SYNCHRONOUS WRITING TO FILE
    return write_to_history(history, MAX_HIST_SIZE);

  } else {
    strncpy(history[(*hist_capacity)++], _input, MAX_INPUT_SIZE);
    // FOR SYNCHRONOUS WRITING TO HIST FILE DURING PROGRAM
    FILE *fp;
    fp = fopen(history_file_dir, "a");
    fflush(fp);
    if (fp == NULL)
      return -1;
    fprintf(fp, "%s\n", _input);
    fclose(fp);
    // memset(history_file_dir, 0, strlen(history_file_dir) * sizeof(char));
  }
  return 0;
}

void print_path() {
  int i = 1;
  char *cur = *environ;

  for (; cur; i++) {
    printf("%s\n", cur);
    cur = *(environ + i);
  }
}

int update_path(char **argv) {
  path temp_path_;
  const char *delim = "=";
  char *token;
  int i = 0;

  while ((token = strsep(&argv[IDENTIFIER_INDEX], delim)) != NULL) {
    if (*token != '\0') {
      if (!i++)
        strncpy(temp_path_.identifier, token, strlen(token) + 1);
      else
        strncpy(temp_path_.body, token, strlen(token) + 1);
    }
  }
  if (i > 0) {
    if (setenv(temp_path_.identifier, temp_path_.body, 1) == -1)
      perror("add export variable failed");
    else
      return SUCCESS;
  }
  return FAILURE;
}

int external_command(char **argv, int argc, int cmd) {
  char *token;
  char *cwd;
  char path_list[MAX_NUM_OF_PATHS_2][MAX_PATH_SIZE];
  int path_list_ind = 0;
  int i = 0;
  int ref = 0;
  int found = 0;
  char *path_ptr = strdup(getenv("PATH"));
  const char *delim = ":";

  // clear buffers
  memset(path_list, 0,
         sizeof(path_list[0][0]) * MAX_NUM_OF_PATHS_2 * MAX_PATH_SIZE);

  if (argv[0][0] == '.') {
    strncpy(path_list[path_list_ind++], getenv("PWD"), strlen(getenv("PWD")));
    ref = 1;
  } else {
    if (path_ptr == NULL || strlen(path_ptr) == 0) {
      return FAILURE;
    }
    while ((token = strsep(&path_ptr, delim)) != NULL) {
      if (*token != '\0') {
        strncpy(path_list[path_list_ind++], token,
                strlen(token) * sizeof(char));
      }
    }
  }

  for (i = 0; i < path_list_ind; i++) {
    if (ref) {
      cwd = strncat(path_list[i], argv[COMMAND_INDEX] + 1,
                    strlen(argv[COMMAND_INDEX]) - 1);
    } else {
      cwd = strncat(path_list[i], "/\0", 2);
      cwd = strncat(cwd, argv[COMMAND_INDEX], strlen(argv[COMMAND_INDEX]) + 1);
    }
    if (access(cwd, 1) == 0) {
      found = 1;
      break;
    }
  }

  free(path_ptr);
  if (found) {
    VLOG(DEBUG, "PATH: %s%s", argv[0], cwd);
    return fork_exec_and_wait(cwd, argv, argc, cmd);
  } else {
    return FAILURE;
  }
}

int write_to_history(char **history, int hist_capacity) {
  // write to file upon exit, the data structure history
  int status = 0;
  FILE *fp;
  fp = fopen(history_file_dir, "w");
  fflush(fp);
  if (fp == NULL)
    return -1;
  int j;
  for (j = 0; j < hist_capacity; j++) {
    // if any ret value is negative, fprintf failed and the func returns -1
    int ret = fprintf(fp, "%s\n", history[j]);
    if (ret < 0)
      status = -1;
  }
  return status;
}

static int redirect_input(char *_file) {
  int fp;
  VLOG(DEBUG, "FLE:%s", _file);
  if ((fp = open(_file, O_RDONLY, 0644)) < 0) {
    perror(_file); /* open failed */
    free(_file);
    return FAILURE;
  }
  dup2(fp, READ);
  close(fp);
  free(_file);
  return SUCCESS;
}

static int redirect_output(char *_file) {
  int fp;
  VLOG(DEBUG, "FLE:%s", _file);
  if ((fp = open(_file, O_WRONLY | O_TRUNC | O_CREAT, 0644)) < 0) {
    perror(_file); /* open failed */
    free(_file);
    return FAILURE;
  }
  dup2(fp, WRITE);
  close(fp);
  free(_file);
  return SUCCESS;
}

int fork_exec_and_wait(char *file, char **argv, int argc, int cmd) {
  if (cmd != ONLY_CMD) {
    argv[argc] = NULL;
    execvp(file, argv);
    perror("exec failed");
    exit(EXIT_FAILURE);
  }
  int child_pid;
  int status = SUCCESS;
  switch ((child_pid = fork())) {
  case CHILD:
    argv[argc] = NULL;
    execvp(file, argv);
    perror("exec failed");
    exit(EXIT_FAILURE);
    break;
  case FORK_FAILED:
    perror("fork failed");
    break;
  default:
    waitpid(child_pid, &status, 0);
    break;
  }
  return status;
}

void debug_array(char **arr, int size, char *id) {
  int i;
  VLOG(DEBUG, "%s START", id);
  for (i = 0; i < size; i++) {
    VLOG(DEBUG, "%s", arr[i]);
  }
  VLOG(DEBUG, "%s END", id);
}
