# a simple "dash" bash-like shell
Jaisal Friedman
Operating Systems Spring 2019
NYU Abu Dhabi

##to-do
2. fix issues with argc[1] cut off on replacement of $
3. fix segfault on exit after $ usage
4. piping multiple processes
5. I/O indirection
6. new README.MD file

## dash
### Loading history
if there is no .history file, the program will notify the user that the load history failed. It will not block the dash program though. Once the program is exited once, the .history file will be created and written to.
### Creating the Path
the path is created using an array of struct path called path_ which stores an array of unique identifiers and a body for each PATH object. The default "PATH" variable is added upon initialization.
### while(!exit) - Main Loop
The loop within main which runs the code for each command from the input proceeds in the following sequence
1. receives the input
2. tokenizes the input
3. checks that the number of tokens is greater than 0
4. checks if the command is a reference to a historical command
5. if 4 is not true, updates the history data structure using the update_history func
6. parses through the built-in commands: cd, pwd, export, history and exit
7. if 6 is not true, checks for an external command using the external_command func using the path_[PATH_INDEX].body elements
8. if 7 is not true, command not found
### Input
the program waits for user input
it gets each line of input and passes it to the parse_input function to tokenize
### Tokenize
the parse_input func in the func.c file tokenizes the input using the strsep C standard lib function
the argv data structure stores the tokens in the 2-D char array
the argc value stores the number of tokens
the COMMAND_INDEX, which is 0, is to do the agv[COMMAND_INDEX] which determines what command the program needs to process
### History
Functions: load_history, update_history and write_to_history
the history will not "save" if the program is not exited using the exit command
### MACROS
- MAX_PATH_ID_SIZE is the max size of the path identifier character array
- MAX_PATH_SIZE is the max size of the path body character array
- MAX_NUM_OF_PATHS is the max number of paths in the path_ array of path structure. Ex. PATH=/usr/bin, JAVA_PATH=/usr/bin/java is 2 paths.
- MAX_NUM_OF_PATHS_2 is the max number of paths in the path.body character array of an individual path struct. Ex. PATH=/usr/bin:/bin is 2 paths.
- MAX_INPUT_SIZE is the max input size of an input line from the user
- define MAX_TOKEN_SIZE is the max number of tokens passed to the argv structure
- define MAX_HIST_SIZE is the max number of history items stored
- define COMMAND_INDEX is the command index of the argv[COMMAND_INDEX], it is the 0th index
- define IDENTIFIER_INDEX is the identifier index of the argv[IDENTIFIER_INDEX], it is the first index
- define PATH_INDEX is the index for the PATH variable in the path_ array of structs.
