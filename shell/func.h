#ifndef FUNC_H_
#define FUNC_H_

#define MAX_INPUT_SIZE 50
#define ARRAY_OF_CHAR_SIZE 2
#define COMMAND_INDEX 0
#define IDENTIFIER_INDEX 1

#define length_of(str) sizeof(str)/sizeof(str[0])

void append(char*, char);
void copy(char* [], char*, int);
void strip(char*, char*, char* []);
int parse_input(char [], char* []);

#endif // FUNC_H_
