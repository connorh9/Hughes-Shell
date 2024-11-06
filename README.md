---
title: Hughes Shell
---

# Hughes Shell

## Overview
Hughes Shell is a custom Unix-based shell made from scratch. The Hughes Shell contains custom built-in functions, while also supporting the ability to run non built-in functions. This project is meant to emulate shells like bash or zsh though it supports less features.

This shell can be ran in interactive more or batch mode. In interactive mode, the shell will read user input and execute commands until told to stop. In batch mode, it will run through commands in a given file. The shell supports redirections, history, variable management, piping, and executing commands.

## Features
### 1. Interactive & Batch Modes
Interactive Mode: The shell prompts for user input and executes the command after parsing it.
Batch Mode: Executes commands from a file, without showing a prompt, for automation.
### 2. Built-in Commands
* `exit`: Manually exits from the shell.
* `cd`: Changes the directory to the directory specified.
* `ls`: Lists contents of the current directory in alphabetical order.
* `export`: Sets environment variables.
* `local`: Sets shell variables.
* `vars`: Lists the local variables and their values.
* `history`: Lists recently used commands, executes past commands, and changes the size of history. "history" will print the n-most recently used commands, "history <<n>n>" will execute the nth command, and "history set n" will set the history size.
### 3. Command Execution
The shell runs external commands by forking from the shell and running the command from that child process. The shell first checks the absolute path for the executable, if it is not found it loops through all the paths from the PATH environment variable, looking for the correct executable.
### Piping in the Shell

This shell supports the use of piping (`|`) to direct the output of one command to the input of another. With this feature, you can chain commands together, allowing the output from one command to be processed by subsequent commands in the pipeline.

### Syntax

```sh
command1 [args] | command2 [args] | command3 [args]...
```
### Example
```sh
ls | wc -l
```

### 5. Redirection
Note: The shell does not support redirections and piping in the same command.

The shell supports various forms of redirection to manage how command results are handled:
* Input: `[optional file discriptor]<file` 
* Output: `[optional file discriptor]>file` 
* Append Output: `[optional file discriptor]>>file`
* Standard Output and Error: `&>file` 
* Appending Standard Output and Error: `&>>file` 

The optional file descriptor will default to the standard file descriptors (0, 1, 2) for stdin, stdout and stderr respectively, if no file descriptor is specified.

### 6. Variable Management
The shell supports the use of both local and environment variables. Both local and environment variables can be set with local and export, respectively, as shown below.
```sh
local VAR=value
```
```sh
export VAR=value
```
These set variables can also be used in the command line to make entering commands more efficient. To reference a variable use $myvar. Environment variables take priority over local variables. The usage can be seen below.
```sh
cd $a
```
Where a is a local variable with value "home". This in the eyes of the shell evaluates to:
```sh
cd home
```

## Development Insights
This project alone has given me so much experience with system-level programming, namely using fork(), execv(), and dup()/dup2() system calls. In efforts to practice good memory management, I gained experience using Valgrind and fsanitize. I was also able to gain experience using gdb to debug my shell.

### Future Enhancements
Some future plans I have with the shell:
* Handle memory more effectively, specifically making sure every piece of allocated memory is freed.
* Removing edge cases: Give my shell the ability to handle all cases instead of a handful of hardcoded cases.
