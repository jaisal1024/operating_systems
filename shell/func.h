#ifndef FUNC_H_
#define FUNC_H_

#define MAX_PATH_ID_SIZE 50
#define MAX_PATH_SIZE 500
#define MAX_NUM_OF_PATHS 10
#define MAX_NUM_OF_PATHS_2 25
#define MAX_INPUT_SIZE 100
#define MAX_TOKEN_SIZE 10
#define MAX_HIST_SIZE 15
#define COMMAND_INDEX 0
#define IDENTIFIER_INDEX 1
#define PATH_INDEX 0

#define length_of(str) sizeof(str) / sizeof(str[0])

typedef struct {
  char identifier[MAX_PATH_ID_SIZE];
  char body[MAX_PATH_SIZE];
} path;

extern void parse_input(char *, char **, int *);
extern int load_history(char **, int *);
extern int update_history(char **, char[], int *);
extern void print_path(path[], int);
extern int update_path(path *, char **, int *);
extern int external_command(path, char **, int, int);

#endif // FUNC_H_
