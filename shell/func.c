#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "func.h"

// void append(char* arr, char c) {
//   printf("%s\n", );
//   int len = strlen(arr);
//   printf("%i\n", len);
//   printf("%c\n", c);
//   arr[len] = c;
// }
void strip(char *arr){
  int i;
  for (i = 0; i < strlen(arr); i++) {
    if (arr[i] < 32){
      arr[i] = '0';
    }
  }
}

int parse_input(char _input[], char *parsed_input[]){
  //declerations
  int success = 1;
  int no_of_spaces = 0;
  /* -------------parsed_input----------------------
  [0] = command
  [1] = directory or PATH variable
  [2] = unused (for now)
  */
  //strip input
  strip(_input);

  //loop over input
  int i;
  char buffer[MAX_INPUT_SIZE];
  for (i=0; i < strlen(_input) && success; i++){
    //parse each character
    char _inp = _input[i];
    //printf("%c\n", _inp);
    switch (_inp) {
      case ' ':
        if (!no_of_spaces && i==0)
          success = 0;
        else if (!no_of_spaces) {
          parsed_input[0] = buffer;
          memset(buffer, 0, sizeof(buffer));
          no_of_spaces+=1;
        } else if (no_of_spaces == 1) {
          parsed_input[1] = buffer;
          memset(buffer, 0, sizeof(buffer));
          no_of_spaces+=1;
        }
        break;
      case '-':
        strcat(buffer, &_inp);
        break;
      case '!':
        if (i!=0)
          success = 0;
        else
          strcat(buffer, &_inp);
        break;
      case '/':
        if (!no_of_spaces)
          success=0;
        strcat(buffer, &_inp);
        break;
      case '0':
        break;
      default:
        if (isdigit(_inp)){
          strcat(buffer, &_inp);
        } else if(isalpha(_inp)){
            strcat(buffer, &_inp);
        }
        else { //non digit, space, dash, or letter encountered. Improper input
          printf("%d\n", _inp);
          success = 0;
        }
        break;
    }
  }
  //buffer is not emptied
  if (strlen(buffer) != 0) {
    if (!parsed_input[0]){
      printf("%s\n", buffer);
        parsed_input[0] = buffer;
    } else if (!parsed_input[1]){
      parsed_input[1] = buffer;
    }
  }
  return success;
}
