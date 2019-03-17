#ifndef FUNC_H_
#define FUNC_H_

#define MAX_PATH_ID_SIZE 50
#define MAX_PATH_SIZE 500
#define MAX_NUM_OF_PATHS 10
#define MAX_NUM_OF_PATHS_2 25
#define MAX_INPUT_SIZE 100
#define MAX_TOKEN_SIZE 10
#define MAX_HIST_SIZE 25
#define COMMAND_INDEX 0
#define IDENTIFIER_INDEX 1
#define PATH_INDEX 0
#define READ 0
#define WRITE 1
#define CHILD 0
#define FORK_FAILED -1
#define FIRST_CMD 0
#define SUCCESS 0
#define FAILURE -1

#define length_of(str) sizeof(str) / sizeof(str[0])

typedef struct {
  char identifier[MAX_PATH_ID_SIZE];
  char body[MAX_PATH_SIZE];
} path;

extern void execute_command(int, char **, char **, int *);
extern int init_dir();
extern void parse_input(char *, char **, int *, const char *);
extern int load_history(char **, int *);
extern int update_history(char **, char[], int *);
extern void print_path();
extern int update_path(char **);
extern int external_command(char **, int);
extern int fork_exec_and_wait(char *, char **);
extern int write_to_history(char **, int);
extern void debug_array(char **, int, char *);

#endif // FUNC_H_
