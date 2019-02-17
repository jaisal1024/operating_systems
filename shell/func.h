#ifndef FUNC_H_
#define FUNC_H_

#define MAX_INPUT_SIZE 100
#define MAX_TOKEN_SIZE 10
#define COMMAND_INDEX 0
#define IDENTIFIER_INDEX 1

#define length_of(str) sizeof(str)/sizeof(str[0])

extern void parse_input(char* , char**, int*);

#endif // FUNC_H_
