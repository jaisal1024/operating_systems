#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "func.h"
#include "common.h"


static void strip(char *, int, char **);

static void strip(char* _input, int argc, char** argv){
  //replace special characters in input
  int i;
  for (i = 0; i < strlen(_input); i++) {
    if (_input[i] < 32){
      _input[i] = '\0';
    }
  }
  //set to empty
  memset(argv, 0, argc*sizeof(argv));
}

void parse_input(char _input [], char** argv, int* argc){
  //declerations
  const char* delim = " ";
  char* token;
  int i;

  /* -------------parsed_input----------------------
  [0] = command
  [1-n] = flags and other identifiers
  */
  //strip input
  //strip(_input, parsed_input);
  //loop over input
  while ((token = strsep(&_input, delim)) != NULL) {
        if (*token != '\0') {
            if (token[strlen(token)-1] < 32){
              token[strlen(token)-1] = '\0';
            }
            argv[(*argc)++]=token;
            VLOG(DEBUG, "T:%s", token);
            VLOG(DEBUG, "C:%i", *argc);
        }
    }
  VLOG(DEBUG, "D1: %s", argv[COMMAND_INDEX]);
}
