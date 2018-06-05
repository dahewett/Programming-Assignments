/*
 * Appendix A program.
 * CSC 360, Summer 2017
 *
 * This shows how a simple loop obtaining input from the user can
 * be written. Notice the use of "fflush".
 *
 * If "impeach" is entered, then the shell terminates.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/wait.h>
#include <unistd.h>

#define MAX_INPUT_LINE 80
#define MAX_NUM_ARGS 7

void cmd_loop(char* input, char** token, int* tp, char* t);
void tokenize (char* input, char** token, int* tp, char* t);
void launch_program (char** token);

int main(int argc, char *argv[]) {

	//char userInput[MAX_INPUT_LINE];
	char input[MAX_INPUT_LINE];
	char *token [MAX_NUM_ARGS];
	char *t;
	//number of tokens (args)
	int numTokens = 0;
	//token pointer
	int *tp;
	tp = &numTokens;
	
	
	//the shells repeating loop
	cmd_loop(input, token, tp, t);
	//printf("/nnumTokens = &d" ,numTokens);
	return 0;
	
	
}
	
void cmd_loop(char* input, char** token, int* tp, char* t ) {
	
	// For knowing the length of each user input
    int  line_len;
	
	// Repeating loop.
    for(;;) {
        fprintf(stdout, "> ");
        fflush(stdout);
        fgets(input, MAX_INPUT_LINE, stdin);
        if (input[strlen(input) - 1] == '\n') {
            input[strlen(input) - 1] = '\0';
        }

        if (strcmp(input, "exit") == 0) {
            exit(0);
		}
		
		tokenize(input, token, tp, t);

		int num_tokens = *tp;
		int i = 0;
		for (i = 0; i < num_tokens; i++) {
			printf("%d: %s\n", i, token[i]);
		} 
		
    } 
		
}
	
void tokenize (char* input, char** token, int* tp, char* t ) {
	
	int i = 0;
	// tokenize input
	*tp = 0;
	int num_tokens = *tp;
	
	//  store in char pointer string token 
	t = strtok(input, " ");
	
	// check for arguments length
	if (num_tokens > MAX_NUM_ARGS) {
		printf("Cannot have more than 7 arguments.");
	}
	//
	while(t != NULL && num_tokens < MAX_NUM_ARGS) {
		token[num_tokens] = t;
		num_tokens++;
		t = strtok(NULL, " ");
	}
	/*
	char *arguments[num_tokens];
	memcpy(arguments, token, sizeof(arguments));

	arg_length = sizeof(arguments) / sizeof(arguments[0]);
	printf("Token Length: %d\n", arg_length);
	*/


	*tp = num_tokens;
}

void launch_program (char** token){
	
	// Initilizing
	pid_t pid;
	pid_t wait_pid;
	int status;
	
	pid = fork();
	
	// child process
	if(pid == 0) {
		printf("child: about to start...\n");
		execve(token[0], 
	
	
	
	