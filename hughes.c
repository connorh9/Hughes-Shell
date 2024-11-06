#include "hughes.h"

char* redirect_file; //tracks the file to be used for redirection

int redirect_val; //stores a value 1-5 which is used to determine the redirect type

int file_descriptor; 

int return_code;

int buf_size = 1; //buffer size for args

int args_count = 0; //used when freeing args

int var_count = 0;
int var_capacity = 0;
LocalVar** local_vars = NULL; //creating an array of structs

CommandHistory* history_list = NULL;

/**
 * Runs if user types exit or EOF is reached.
 * Frees allocated memory before exiting with the global return code
*/
void h_exit(char** args){
	for(int i = 0; i < history_list -> count; i++){
		free(history_list -> commands[i]);
	}
	free(history_list -> commands);
	free(history_list);
	if(local_vars != NULL){
		free_locals();
	}
	for(int i = 0; i < args_count; i++;){
		free(args[i]);
	}
	free(args);
	exit(return_code);
	
}
/**
 * Prints out all the local variables
 * and their corresponding values
 * Params- args, used to check that vars was not ran
 * with more than one argument
 * */
void vars(char** args){
	if(args[1] != NULL){
		fprintf(stderr, "vars takes no args");
		return_code = 1;
		return;
	}
	if(var_count == 0){
		fprintf(stderr, "No variables");
		return_code = 1;
		return;
	}
	for(int i = 0; i < var_count; i++){
		printf("%s=%s\n", local_vars[i] -> name, local_vars[i] -> value);
	}
	return_code = 0;
	return;
}

/**
 * Handles setting and replacing local variable values
 * The argument containing the variable assignment is 
 * hardcoded to only accept variable assignment in the 
 * second index of the split.
*/
void local(char** args){
	char* name = strdup(args[1]);
	char* equalSign = strchr(name, '=');
	if(equalSign == NULL){
		free(name);
		free(equalSign);
		exit(1);
	}
	//Splits the left and right side of equal sign
	*equalSign = '\0';
	char* value = equalSign + 1;

	//check if local_vars is initialized
	if(var_capacity == 0){
		var_capacity = 10;
		local_vars = malloc(var_capacity * sizeof(LocalVar*));
		if(local_vars == NULL){
			fprintf(stderr,"couldnt allocate local vars");
			free(name);
			free(equalSign);
			exit(1);
		}
	}
	else if(var_count >= var_capacity){
		//Realloc if necessary
		var_capacity *= 2;
		local_vars = realloc(local_vars, var_capacity * sizeof(LocalVar*));
		if(local_vars == NULL){
			fprintf(stderr, "Couldnt allocate local vars");
			free(name);
			free(equalSign);
			exit(1);
		}
	}
	//Check if the value to be set is already a variable, if so handle it
	if(value[0] == '$'){
		int foundFlag = 1;
		value = equalSign +2;
		for(int j = 0; j < var_count; j++){
			if(strcmp(local_vars[j] -> value, value) == 0){
				foundFlag = 0;
				value = strdup(local_vars[j] -> value);
			}
		}
		if(foundFlag == 1){
			value = strdup("");
		}
	}
	for(int i = 0; i < var_count; i++){
		if(strcmp(local_vars[i] -> name, name) == 0){
			local_vars[i] -> value = strdup(value);
			return_code = 0;
			free(name);
			free(equalSign);
			return;
		}
	}
	
	local_vars[var_count] = malloc(sizeof(LocalVar));
	if(local_vars[var_count] == NULL){
		fprintf(stderr, "Couldnt alloc local var");
		free(name);
		free(equalSign);
		exit(1);
	}
	local_vars[var_count] -> name = strdup(name);
	local_vars[var_count] -> value = strdup(value);
	var_count++;
	return_code = 0;
	free(name);
	free(equalSign);
	return;
}

//Helper method to free local variables
void free_locals(){
	for(int i = 0; i < var_count; i++){
		free(local_vars[i] -> name);
		free(local_vars[i] -> value);
		free(local_vars[i]);
	}
	free(local_vars);
}

//Helper method to retrieve local variable value
char* get_local_value(char* name){
	for(int i = 0; i < var_count; i++){
		if(strcmp(local_vars[i] -> name, name) == 0){
			return local_vars[i] -> value;
		}
	}
	return NULL;
}

/**
 * Sets an environment variable
 * format is export variable=value
*/
void export(char** args){
	char* secondArg= strdup(args[1]);
	char* ptrToEquals = strchr(secondArg, '=');
	if(ptrToEquals == NULL){
		free(secondArg);
		exit(1);
	}
	*ptrToEquals = '\0';
	char* var = secondArg;
	char* val = ptrToEquals + 1;

	if(setenv(var, val, 1) != 0){
		fprintf(stderr, "Error setting env");
		return_code = 1;
		free(secondArg);
		free(ptrToEquals);
		return;
	}
	return_code = 0;
	return;
}

//Change directory to specified directory
void cd(char** args){
	if(args[1] == NULL){
		fprintf(stderr, "cd expects second arg\n");
		return_code = 1;
		return;
	}
	else if(args[2] != NULL){
		fprintf(stderr, "cd only takes 2 arguments\n");
		return_code = 1;
		return;
	}
	else{
		if(chdir(args[1]) != 0){
			fprintf(stderr, "chdir error\n");
			return_code = 1;
			return;
		}
		return_code = 0;
		return;
	}
}

//Helper method to initialize history at startup, default size is 5
int init_history(int size){
	history_list = malloc(sizeof(CommandHistory));
	if(history_list == NULL){
		fprintf(stderr, "Could not allocate history");
		exit(1);
	}

	history_list -> commands = malloc(size * sizeof(char*));
	if(history_list -> commands == NULL){
		fprintf(stderr, "Could not allocate command history");
		exit(1);
	}

	history_list -> capacity = size;
	history_list -> count = 0;
	return_code = 0;
	return 1;
}

/**
 * This function is called when history is the 
 * first argument in the args array. We then decide which history command we
 * need to run based on the number of arguments passed in.
*/
void history(char** args){
	int num_params = 0;
	while(args[num_params] != NULL){
		num_params++;
	}
	if(num_params == 1){
		print_history();
	}
	else if(num_params == 2){
		exec_history(args);
	}
	else if(num_params == 3){
		set_capacity(atoi(args[num_params-1]));
	}
	else{
		fprintf(stderr, "history error\n");
		return_code = 1;
		return;
	}
	return_code = 0;
	return;
}

/**
 * Helper method which increases or decreases the capacity
 * of the history, and adjusts the history list as necessary
*/
void set_capacity(int newCapacity){
	if(newCapacity <= 0){
		fprintf(stderr, "Invalid capacity\n");
		return_code = 1;
		return;
	}
	if(newCapacity < history_list -> capacity){
		for(int i = newCapacity; i < history_list -> count; i++){
			free(history_list -> commands[i]);
		}
		history_list -> count = newCapacity;
	}
	history_list -> commands = realloc(history_list -> commands, newCapacity * sizeof(char*));
	history_list -> capacity = newCapacity;
	return_code = 0;	
}

void print_history(){
	for(int i = 0; i < history_list -> count; i++){
		printf("%d) %s\n", i+1, history_list->commands[i]);
	}
	return_code = 0;
}

/**
 * Reads the command from the chosen history slot
 * and creates an arguments array formatted for run_exec
 * to handle.
*/
void exec_history(char** args){
	if(args[1] == NULL){
		fprintf(stderr, "Select a valid history command.\n");
		return_code = 1;
		return;
	}
	int commandNum = atoi(args[1]);
	if(commandNum < 1 ||  commandNum > history_list -> count){
		fprintf(stderr, "Enter a valid selection\n");
		return_code = 1;
		return;
	}

	char* selectedCommand = strdup(history_list -> commands[commandNum-1]);
	char* tempCommand = strdup(selectedCommand);
	char* token = strtok(tempCommand, " ");
	int tokenCount = 0;

	while(token != NULL){
		tokenCount++;
		token = strtok(NULL, " ");
	}
	
	free(tempCommand);
	char** commandToRun = malloc(sizeof(char*) * tokenCount);
	token = strtok(selectedCommand, " ");
	tokenCount = 0;

	while(token != NULL){
		commandToRun[tokenCount] = strdup(token);
		tokenCount++;
		token = strtok(NULL, " ");
	}
	commandToRun[tokenCount]= NULL;
	run_exec(commandToRun);
	free(selectedCommand);
	for(int i = 0; i < tokenCount; i++){
		free(commandToRun[i]);
	}
	free(commandToRun);
	return_code = 0;
	return;	
}

/**
 * Adds the most recent non-built-in command to the front
 * of the history list.
*/
void add_history(char** args){
	size_t commandSize = 0;
	int i = 0;

	while(args[i] != NULL){
		commandSize += strlen(args[i]) + 1;
		i++;
	}
	char* command = malloc(commandSize);

	if(command == NULL){
		fprintf(stderr, "command malloc\n");
		return_code =1;
		h_exit(args);
	}

	command[0] = '\0';

	for(int j = 0; j < i; j++){
		strcat(command, args[j]);
		if(args[j+1] != NULL){
			strcat(command, " ");
		}
	}
	
	if(history_list -> count > 0){
		if(strcmp(history_list -> commands[0], command) == 0){
			return_code = 0;
			free(command);
			return;
		}
	}
	
	if(history_list -> count == history_list -> capacity){
		free(history_list -> commands[history_list -> capacity-1]);
	}

	for(int k = history_list -> capacity -1; k >0; k--){
		history_list -> commands[k] = history_list -> commands[k-1];
	}
	history_list -> commands[0] = strdup(command);

	if(history_list -> count < history_list -> capacity){
		history_list -> count++;
	}
	free(command);
	return_code = 0;
	return;
}

/**
 * prints all files in the current directory.
 * Prints with the format ls LANG=C.
 * ls does not accept flags like -a or -l
*/
void ls(char** args){
	if(args[1] != NULL){
		fprintf(stderr, "ls expects no arguments\n");
		return_code = 1;
		return;
	}

	struct dirent** files;
	//Sorts entries alphabetically 
	int entries = scandir(".", &files, NULL, alphasort);
	if(entries==-1){
		fprintf(stderr,"scandir error\n");
		exit(1);
	}
	
	for(int i =0; i < entries; i++){
		if(files[i] -> d_name[0] != '.'){
			printf("%s\n", files[i] -> d_name);
		}
		free(files[i]);
	}
	free(files);
	return_code = 0;
	return;
}

/**
 * This function handles all the piping
 * and runs run_exec after setting the correct file descriptors.
*/
void run_pipe(char** args) {
	int pipefd[2];
	int i = 0;
	char* left_args[buf_size];
	char* right_args[buf_size];
	int left_index = 0;
	int right_index = 0;
	bool split = false;

	// Split arguments into two arrays at the pipe symbol
	for (; args[i] != NULL; i++) {
		if (strcmp(args[i], "|") == 0) {
			split = true;
			continue;
		}
		if (!split) {
			left_args[left_index++] = args[i];
		} else {
			right_args[right_index++] = args[i];
		}
	}
	left_args[left_index] = NULL;
	right_args[right_index] = NULL;

	// Create the pipe
	if (pipe(pipefd) == -1) {
		perror("pipe");
		exit(1);
	}

	pid_t pid = fork();
	if (pid == 0) { // Child process
		dup2(pipefd[1], STDOUT_FILENO); // Redirect stdout to pipe
		close(pipefd[0]);
		close(pipefd[1]);
		run_args(left_args);
		exit(return_code);
	} else if (pid < 0) {
		perror("fork");
		exit(1);
	}

	// Parent process: Handle the second command in the pipe
	pid_t pid2 = fork();
	if (pid2 == 0) { // Second child process
		dup2(pipefd[0], STDIN_FILENO); // Redirect stdin to pipe
		close(pipefd[0]);
		close(pipefd[1]);
		run_args(right_args);
		exit(return_code);
	} else if (pid2 < 0) {
		perror("fork");
		exit(1);
	}

	// Close pipe and wait for both children
	close(pipefd[0]);
	close(pipefd[1]);
	waitpid(pid, NULL, 0);
	waitpid(pid2, NULL, 0);
	return;
}

/**
 * Runs our non built in commands.
 * We ignore comments in the shell.
 * run_exec first checks the absolute path,
 * if the command is not found there then it checks 
 * every path in the environment PATH variable.
 * We set the global return code based on the
 * result of running the execv command.
*/
int run_exec(char** args){
	if(strcmp(args[0], "#") == 0){
		return_code = 0;
		return 0;
	}
	int rc = fork();
	char* path = NULL;
	if(rc < 0){
		fprintf(stderr, "Could not fork child\n");
		exit(1);
	}
	else if(rc == 0){
		if(access(args[0], X_OK) == 0){
			execv(args[0], args);
			fprintf(stderr, "Could not execute\n");
			return_code = 255;
			exit(255);
		}
		else{
			char* envPath = getenv("PATH");
			char* pathCopy = strdup(envPath);
			if(pathCopy == NULL){
				fprintf(stderr, "strdup error\n");
				exit(1);
			}

			char* token = strtok(pathCopy, ":");
			int pathCount = 0;
			while(token != NULL){
				pathCount++;
				path = malloc(strlen(token) + 1 + strlen(args[0]) + 1);
				strcpy(path, token);
				strcat(path, "/");
				strcat(path, args[0]);
				if(access(path, X_OK) == 0){
					execv(path, args);
					free(pathCopy);
					free(path);
					exit(255);
				}
				token = strtok(NULL, ":");
			}
			free(pathCopy);
			free(path);
			return_code = 255;
			exit(255);
		}
	}
	else{
		int status;
		waitpid(rc, &status, 0);  // Wait for child process

		if (WIFEXITED(status)) {
			return_code = WEXITSTATUS(status);
		}
		else{
			return_code = 1;
		}
	}
	return return_code;	
}
/**
 * Checks for pipes and redirection and decides whether to
 * run a built in command, run the pipe helper method, or
 * run_exec.
*/
int run_args(char** args){
	int fd;
	int oldStdout = STDOUT_FILENO;
	int oldStderr = STDERR_FILENO;
	int oldStdin = STDIN_FILENO;
	int oldFD = file_descriptor;

	//Checks for pipes
	for (int i = 0; args[i] != NULL; i++) {
		if (strcmp(args[i], "|") == 0) {
			run_pipe(args);
			return return_code;
		}
	}

	//Redirection was checked in parse_args, here we set the file descriptors
	//0 = no redirect
	//1 = STDIN redirection
	//2 = STDOUT redirection without append
	//3 = STDOUT redirection with append
	//4 = STDOUT and STDERR redirection without append
	//5 = STDOUT AND STDERR redirection with append
	if(redirect_val != 0){
		if(redirect_val == 1){
			fd = open(redirect_file, O_RDONLY);
			if(fd == -1){
				fprintf(stderr, "Open error\n");
				exit(1);
			}
			dup2(fd, STDIN_FILENO);
			close(fd);
		}
		if(redirect_val == 2){
			fd = open(redirect_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
			if(fd == -1){
				fprintf(stderr, "Open error\n");
				exit(1);
			}
			if(file_descriptor != -1){
				dup2(fd, file_descriptor);
				close(fd);
			}
			else{
				dup2(fd, STDOUT_FILENO);
				close(fd);
			}
			//close(fd);
		}
		if(redirect_val == 3){
			fd = open(redirect_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
			if(fd == -1){
				fprintf(stderr, "Open error\n");
				exit(1);
			}
			if(file_descriptor != -1){
				dup2(fd, file_descriptor);
				close(fd);
			}
			else{
				dup2(fd, STDOUT_FILENO);
				close(fd);
			}
		}
		if(redirect_val == 4){
			fd = open(redirect_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
			if(fd == -1){
				fprintf(stderr, "Open error\n");
				exit(1);
			}
			dup2(fd, STDERR_FILENO);
			dup2(fd, STDOUT_FILENO);
			close(fd);
		}
		if(redirect_val == 5){
			fd = open(redirect_file, O_WRONLY| O_CREAT | O_APPEND, 0644);
			if(fd == -1){
				fprintf(stderr, "Open error\n");
				exit(1);
			}
			dup2(fd, STDOUT_FILENO);
			dup2(fd, STDERR_FILENO);
			close(fd);
		}
	}

	//Check if we need to run builtin functions
	if(strcmp(args[0], "exit") == 0){
		h_exit(args);
	}
	else if(strcmp(args[0], "cd") == 0){
		cd(args);
	}
	else if(strcmp(args[0], "export") == 0){
		export(args);
	}
	else if(strcmp(args[0], "local") == 0){
		local(args);
	}
	else if(strcmp(args[0], "vars") == 0){
		vars(args);
	}
	else if(strcmp(args[0], "history") == 0){
		history(args);
	}
	else if(strcmp(args[0], "ls") == 0){
		ls(args);
	}
	else{
		run_exec(args);
	}
	
	//Restore original file descriptors
	if(file_descriptor != -1){
		dup2(oldFD, file_descriptor);
	}
	dup2(oldStdin, STDIN_FILENO);
	dup2(oldStdout, STDOUT_FILENO);
	dup2(oldStderr, STDERR_FILENO);
	close(oldStdin);
	close(oldStdout);
	close(oldStderr);
	file_descriptor = -1;
	redirect_val = 0;
	return return_code; 
}

/**
 * Parses the input by spaces and forms an array of char pointers.
 * Also checks for the redirection type and file descriptor.
*/
char **parse_line(char* line){
	if(strcmp(line, "") == 0 || strcmp(line, "\n") == 0) {
		return NULL;
	}
	buf_size = 1;
	int argPosition = 0;
	char** args = malloc(sizeof(char*) * buf_size);
	if(args == NULL){
		fprintf(stderr, "Could not allocate buffer for args\n");
		exit(1);
	}
	char* currToken = strtok(line, delimiters);
	
	while(currToken != NULL){
		if(currToken[0]=='#'){
			args[0] = "#";
			return args;
		}
		if(currToken[0]	== '$'){
			char* name = currToken +1;
			if(getenv(name) != NULL){
				currToken = getenv(name);
			}
			else{
				char* val = get_local_value(name);	
				if(val != NULL){
					currToken = strdup(val);
				}
				else{
					currToken = strdup("");
				}
			}
		}
		args[argPosition] = strdup(currToken);
		argPosition++;
		
		if(argPosition >= buf_size){
			buf_size = buf_size + 1;
			args = realloc(args, buf_size * sizeof(char*));
			
			if(args == NULL){
				fprintf(stderr, "Could not allocate buffer for args\n");
				h_exit(args);
			}
		}
		args_count++;
		currToken=strtok(NULL, delimiters);
	}
	args[argPosition] = NULL;
	free(currToken);
	char* finalToken = args[argPosition-1];
	char* descriptor = strdup(finalToken);
	char* oldDesc = descriptor;

	//Checks if a file descriptor was specified for redirection
	if(isdigit(finalToken[0])){
		int i = 0;
		while(isdigit(finalToken[i])){
			i++;
		}
		descriptor[i] = '\0';
		file_descriptor = atoi(descriptor);
		descriptor = descriptor + i; 
	}
	else{
		file_descriptor = -1;
	}
	//Determines which type of redirection we will do
	if(strstr(finalToken, "&>>")){
		redirect_val = 5;
		redirect_file = strdup(descriptor + 3);
	}
	else if(strstr(finalToken, "&>")){
		redirect_val = 4;
		redirect_file = strdup(descriptor+2);
	}
	else if(strstr(finalToken, ">>")){
		redirect_val = 3;
		redirect_file = strdup(descriptor+2);
	}
	else if(strstr(finalToken, ">")){
		redirect_val = 2;
		redirect_file = strdup(descriptor+1);
	}
	else if(strstr(finalToken, "<")){
		redirect_val = 1;
		redirect_file = strdup(descriptor+1);
	}

	free(oldDesc);
	
	//Ensures the command is not a built in before adding it to history
	if(strcmp(args[0], "exit") != 0 && strcmp(args[0], "local") != 0 && strcmp(args[0], "cd") != 0 &&
		strcmp(args[0], "export") != 0 && strcmp(args[0], "vars") != 0 && strcmp(args[0], "ls") != 0 && strcmp(args[0], "history") != 0){
			add_history(args);
	}
	//Remove the redirection from the arguments array
	if(redirect_val && redirect_file){
		args[argPosition - 1] = '\0';
	}
	return args;
}	

int main(int argc, char* argv[]){
	if(argc > 2){
		exit(1);
	}
	return_code = 0;
	bool batchMode = true;

	//Path environment variable should begin as /bin
	if(setenv("PATH", "/bin", 1) != 0){
		fprintf(stderr, "Error setting env\n");
		exit(1);
	}
	//If hughes.c is ran with more than 1 arg and less than 3 then we are in batch mode
	if(argc == 1){
		batchMode = false;
	}
	
	init_history(default_history_size);
	char** args;
	args_count = 0;
	
	if(batchMode){
		FILE *script = fopen(argv[1], "r");
		if(script == NULL){
			fprintf(stderr, "Couldnt open script\n");
			exit(1);
		}
		char* batchLine = NULL;
		size_t batchSize = 0;
		fflush(stdout);
		while(1){
			if(getline(&batchLine, &batchSize, script) != -1){
				char** args = parse_line(batchLine);
				if(args != NULL){
					run_args(args);
				}
			}
			else{
				free(batchLine);
				fclose(script);
				h_exit(args);
			}
			for(int i = 0; i < args_count; i++;){
				free(args[i]);
			}
			args_count = 0;
		}
	}
	else{
		while(1){
			printf("%s", "wsh> ");
			fflush(stdout);
			char* line = NULL;
			size_t getlineBufsize = 0;

			if(getline(&line, &getlineBufsize, stdin) != -1){
				args = parse_line(line);
				if(args != NULL){
					run_args(args);	
				}
			}
			else{
				if(feof(stdin)){
					free(line);
					h_exit(args);
				}
				else{
					free(line);
					return_code =1;
					h_exit(args);
				}
			}
			for(int i = 0; i < args_count; i++;){
				free(args[i]);
			}
			args_count = 0;
		}
	}
	return 1;
}

