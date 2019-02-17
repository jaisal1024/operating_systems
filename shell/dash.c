#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "func.h"
#include "common.h"

int main(int argc, char const *argv[]) {
  int exit = 0;
  char wd[1000];
  while (!exit) {
    //declerations
    char _input[MAX_INPUT_SIZE];
    char * parsed_input[ARRAY_OF_CHAR_SIZE];
    // char *test = (char *) malloc(MAX_INPUT_SIZE * sizeof(char));
    int i;
    for (i=0; i<ARRAY_OF_CHAR_SIZE; i++){
      parsed_input[i] = (char *)malloc(MAX_INPUT_SIZE*sizeof(char));
    }
    //VLOG(DEBUG, "size of %lu", length_of(*parsed_input[COMMAND_INDEX]));
    VLOG(DEBUG, "%i", strncmp("WD", "WD",2)==0); //this is because of the weird 1st round error
    //get input
    printf(">>");
    fgets(_input, MAX_INPUT_SIZE, stdin);
    //parse input and check output
    int successful_inputs = parse_input(_input, parsed_input);
    if (successful_inputs) {
        VLOG(DEBUG, "D1: %s", parsed_input[IDENTIFIER_INDEX]);
        VLOG(DEBUG, "D2: %s", parsed_input[COMMAND_INDEX]);
        // VLOG(DEBUG, "D2: %i", strncmp(parsed_input[i], "pwd", 3));
        if (strncmp(parsed_input[COMMAND_INDEX], "exit", 4)==0){
          VLOG(INFO, "Dash: bye");
          //exit(0)
          exit = 1;
        } else if(strncmp(parsed_input[COMMAND_INDEX], "pwd", 3)==0){
          //if (!wd)
          getwd(wd);
          VLOG(INFO, "%s", wd);
        } else if (strncmp(parsed_input[COMMAND_INDEX], "cd", 2)==0) {
          if (successful_inputs < 2){
            VLOG(WARNING, "please provide a path ");
          } else {
            getwd(wd);
            char * cwd = strcat(wd, "/\0");
            cwd = strcat(cwd, parsed_input[IDENTIFIER_INDEX]);
            if (access(cwd, 1)==0) {
              if (chdir(cwd) == -1) {
                VLOG(WARNING, "access denied");
              }
            } else {
              VLOG(WARNING, "change directory failed");
            }
          }
        } else {
          VLOG(WARNING, "command not found");
        }
    } else {
      VLOG(INFO, "command not found");
      // printf("\n");
    }
    for (i=0; i<ARRAY_OF_CHAR_SIZE; i++){
      free(parsed_input[i]);
    }
  }
  return 0;
}
