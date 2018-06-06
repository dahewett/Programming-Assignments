/*
 * sh360.c 
 * Devin Hewett V00821219
 * CSC 360, Summer 18
 *
 * Simple unix-like shell.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/wait.h>
#include <unistd.h>

#define MAX_INPUT_LINE 80
#define MAX_NUM_ARGS 7


void cmd_loop(char* input, int* tp, char* t);
void tokenize (char* input, char** token, int* tp, char* t);
void launch_program (char** token);
void execute_commands (char** token, int* tp);

int main(int argc, char *argv[]) {

	//char userInput[MAX_INPUT_LINE];
	char input[MAX_INPUT_LINE];

	char *t;
	//number of tokens (args)
	int numTokens = 0;
	//token pointer
	int *tp;
	tp = &numTokens;
	
	
	//the shells repeating loop
	cmd_loop(input, tp, t);
	//printf("/nnumTokens = &d" ,numTokens);
	return 0;
}
	
void cmd_loop(char* input, int* tp, char* t ) {
	
	// For knowing the length of each user input
    int  line_len;
	int loop = 0;
	
	// Repeating loop.
    while (!loop) {
		char *token [MAX_NUM_ARGS];
        fprintf(stdout, "> ");
        fflush(stdout);
        fgets(input, MAX_INPUT_LINE, stdin);
        if (input[strlen(input) - 1] == '\n') {
            input[strlen(input) - 1] = '\0';
        }
		
		tokenize(input, token, tp, t);
		execute_commands(token, tp);

		int num_tokens = *tp;
    } 
}
	
void tokenize (char* input, char** token, int* tp, char* t ) {
	

	// tokenize input
	*tp = 0;
	int num_tokens = *tp;
	
	//  store in char pointer string token 
	t = strtok(input, " ");
	
	// check for arguments length
	if (num_tokens > MAX_NUM_ARGS) {
		fprintf(stderr, "Cannot have more than 7 arguments./n");
	}
	//
	while(t != NULL && num_tokens < MAX_NUM_ARGS) {
		token[num_tokens] = t;
		num_tokens++;
		t = strtok(NULL, " ");
	}
	
	*tp = num_tokens;

}

void launch_program (char** token){
	
	// Initilizing
	char* envp[] = {0};
	pid_t pid;
	int status;
	
	// process fork
	pid = fork();
	
	// child process
	if(pid == 0) {
		printf("child: about to start...\n");
		execve(token[0], token, envp);
		fprintf(stderr, "Command not recognized.\n");
		
	} else if (pid < 0) {
		fprintf(stderr, "Error forking.\n");
		
	} 
	waitpid(pid, &status, 0);
		
}

void execute_commands (char** token, int* tp) {
	
	int length_args = *tp;
	
	if ( length_args == 0) {
		printf("No arg entered\n");
	} 
	// Built-in commands:
	// for changeing directories (cd)
	else if ( strcmp(token[0], "cd") == 0) {
		int check;
		check = chdir(token[1]);
		if(check != 0) {
			fprintf(stderr, "cd: argument not recognized.\n");
		}
	}
	// Display current directory 
	else if (strcmp(token[0], "pwd") == 0) {
		char directory[60];
		getcwd(directory, 60);
		printf("Current working directory: %s\n", directory);
		
	}
	// Exit program
	else if (!strcmp(token[0], "exit")) {
            exit(0);
	}else {
		launch_program(token);
	}
}
	
	
	
	
	
	
	