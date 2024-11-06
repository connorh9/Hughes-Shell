#include "fcntl.h"
#include "ctype.h"
#include "unistd.h"
#include "stdio.h"
#include "stdlib.h"
#include "stdbool.h"
#include "string.h"
#include "sys/wait.h"
#include "dirent.h"

#define delimiters " \t\r\n\a"
#define default_history_size 5

void h_exit(char** args);
void cd(char** args);
void export(char** args);
void local(char** args);
void vars(char** args);
void ls(char** args);
int run_exec(char** args);
void free_locals();
char** parse_line(char* line);
int run_args(char** args);
int init_history(int size);
void free_locals();
char* get_local_value(char* name);
void add_history(char** args);
void exec_history(char** args);
void print_history();
void set_capacity(int newCapacity);
void run_pipe(char** args);

typedef struct CommandHistory{
	char** commands;
	int capacity;
	int count;
} CommandHistory;	

typedef struct LocalVar{
	char* name;
	char* value;
} LocalVar;