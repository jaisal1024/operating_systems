#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "func.h"

int main(int argc, char const *argv[]) {
  int exit = 0;
  while (!exit) {
    //declerations
    char _input[MAX_INPUT_SIZE];
    char *parsed_input[ARRAY_OF_CHAR_SIZE];
    //get input
    fgets(_input, MAX_INPUT_SIZE, stdin);
    //parse input and check output
    if (parse_input(_input, parsed_input)) {
      int i;
      for (i = 0; i < MAX_INPUT_SIZE && strlen(parsed_input[i])!= 0; i++) {
        //printf("%s", parsed_input[i]);
        //printf("\n");
      }
    } else {
      printf("%s: command certaintly not found", _input);
      printf("\n");
    }
  }
  return 0;
}
