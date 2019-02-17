#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "func.h"
#include "common.h"


static void append(char *, char);
static void copy(char *parsed_input[], char *buffer, int index);

void append(char *buffer, char i){
  char _i[2]= " \0";
  _i[0]=i;
  strcat(buffer,_i);
}
void copy(char *parsed_input[], char *buffer, int index){
  int n = strlen(buffer);
  parsed_input[index] = realloc(parsed_input[index], n);
  strncpy(parsed_input[index], buffer, n);
  parsed_input[index][n]='\0';
}
void strip(char *_input, char* buffer, char* parsed_input[]){
  //replace special characters in input
  int i;
  for (i = 0; i < strlen(_input); i++) {
    if (_input[i] < 32){
      _input[i] = '\0';
    }
  }
  //set to empty
  memset(buffer, 0, sizeof(&buffer));
  memset(parsed_input, 0, sizeof(&parsed_input));
}

int parse_input(char _input[], char *parsed_input[]){
  //declerations
  int success = 1;
  int no_of_spaces = 0;
  int no_of_inputs = 0;
  int i;
  char buffer[MAX_INPUT_SIZE];
  /* -------------parsed_input----------------------
  [0] = command
  [1] = directory or PATH variable
  [2] = unused (for now)
  */
  //strip input
  strip(_input, buffer, parsed_input);
  //loop over input
  for (i=0; i < strlen(_input) && success; i++){
    //parse each character
    char _inp = _input[i];
    switch (_inp) {
      case ' ':
        if (!no_of_spaces && i==0)
          success = 0;
        else if (!no_of_spaces) {
          copy(parsed_input, buffer, COMMAND_INDEX);
          memset(buffer, 0, sizeof(buffer));
          no_of_spaces = no_of_spaces+1;
        } else if (no_of_spaces == 1) {
          copy(parsed_input, buffer, IDENTIFIER_INDEX);
          memset(buffer, 0, sizeof(buffer));
          no_of_spaces = no_of_spaces+1;
        }
        break;
      case '-':
        append(buffer, _inp);
        break;
      case '_':
        append(buffer, _inp);
        break;
      case '!':
        if (i!=0)
          success = 0;
        else
          append(buffer, _inp);
        break;
      case '/':
        if (!no_of_spaces)
          success=0;
        append(buffer, _inp);
        break;
      case '0':
        break;
      default:
        if (isdigit(_inp)){
          append(buffer, _inp);
        } else if(isalpha(_inp)){
          append(buffer, _inp);
        }
        else { //non digit, space, dash, or letter encountered. Improper input
          VLOG(DEBUG, "%d", _inp);
          success = 0;
        }
        break;
    }
  }
  //buffer is not emptied
  if (strlen(buffer) != 0) {
    if (!no_of_spaces){
      copy(parsed_input, buffer, COMMAND_INDEX);
    } else if (no_of_spaces==1){
      copy(parsed_input, buffer, IDENTIFIER_INDEX);
    }
  }
  if (success)
    return no_of_spaces+1;
  else
    return success;
}
