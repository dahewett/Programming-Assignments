/*
 * sh360.c 
 *
 * Devin Hewett V00821219
 * CSC 360, Summer 2018
 * Assignment 1; Due: June 6, 2018
 *
 * Simple unix-like shell. With piping and output redirection.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>


#define MAX_INPUT_LINE 80
#define MAX_NUM_ARGS 7
#define MAX_PROMPT 10

// initializes functions
void cmd_loop(char* input, char** token, int* tp, char* t);
void tokenize (char* input, char** token, int* tp, char* t);
int launch_program (char** token);
void execute_commands (char** token, int* tp);
void read_file();
void redirecting_output(char** token, int* tp);
void create_arrow_index (char** token, int length_args);
void prepend_arg(char** token, int rc);
int piping(char** token, int* tp);
void p_create_arrow_index(char** token, int length_args);


// global vars
char prompt [MAX_PROMPT];
char direct [10][MAX_INPUT_LINE];
int arrows_ind [MAX_NUM_ARGS];
int rc_counter = 1;
char rc_arg [MAX_INPUT_LINE];
int p_arrows_ind [MAX_NUM_ARGS];

int main(int argc, char *argv[]) {

	// initialize
	char input[MAX_INPUT_LINE];
	char *token [MAX_NUM_ARGS];
	char *t;
	//number of tokens (args)
	int numTokens = 0;
	//number of tokens pointer
	int *tp;
	tp = &numTokens;
	
	
	//the shells repeating loop
	cmd_loop(input, token, tp, t);
	return 0;
}
	
void cmd_loop(char* input, char** token, int* tp, char* t ) {
	
	// For knowing the length of each user input
    int  line_len;
	int loop = 0;
	
	read_file();
	
	// Repeating loop.
    while (!loop) {
		
		// display prompt.
        fprintf(stdout, "%s ", prompt);
        fflush(stdout);
        fgets(input, MAX_INPUT_LINE, stdin);
		// change new line to null.
        if (input[strlen(input) - 1] == '\n') {
            input[strlen(input) - 1] = '\0';
        }
		
		// calls fucntion to tokenize the input.
		tokenize(input, token, tp, t);
		
		// execute built in commands and others.
		execute_commands(token, tp);

		int num_tokens = *tp;
		if(strcmp(input, "exit") == 0) {
            loop = 1;
			break;
			exit(0);
        }
    } 
}
// Reads rc file and updates prompt, stores rest of rc content in a 2d array sorted by line.
// adapted from: https://stackoverflow.com/questions/3463426/in-c-how-should-i-read-a-text-file-and-print-all-strings
void read_file () {
	
	FILE *file_pointer;
	if((file_pointer = fopen(".sh360rc", "r")) == 0) {
		fprintf (stderr, "File not found.");
	}
	
	fgets(prompt, MAX_PROMPT, file_pointer);
	
	if( prompt[strlen(prompt) - 1] == '\n') {
		prompt[strlen(prompt) - 1] = '\0';
		
	}
	
	int i = 0;
	
	while (fgets(direct[i], sizeof(direct[i]), file_pointer) && i < 10) {
		rc_counter++;
		// Use a char pointer to go through the 2D array
		char* dirP;
		dirP = direct[i];
		
		if( dirP[strlen(dirP) - 1] == '\n') {
			dirP[strlen(dirP) - 1] = '\0'; 
		}
		i++;
	}
}
	
// tokenizes user input
// adapted from: appendix_b
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
// adapted from: https://brennan.io/2015/01/16/write-a-shell-in-c/
int launch_program (char** token){
	
	// Initilizing
	char* envp[] = {0};
	pid_t pid;
	int status;
	// check all of rc direct array
	int i;

	for (i = 0; i < rc_counter-1; i++) {
		prepend_arg(token, i);
		// process fork
		pid = fork();
	
		// child process
		if(pid == 0) {

			execve(rc_arg, token, envp);
			continue;
			
		} else if (pid < 0) {
			fprintf(stderr, "Error forking.\n");
			
		} 
		waitpid(pid, &status, 0);
		return 0;
	}
	fprintf(stderr, "Command not recognized.\n");
	return 1;
		
}
// appending index of rc to token
void prepend_arg (char** token, int rc) {
	
	
	stpcpy(rc_arg, direct[rc]);
	strcat(rc_arg, "/");
	strcat(rc_arg, token[0]);
	
}
// Executes built in commands else 
// Adapted from: https://brennan.io/2015/01/16/write-a-shell-in-c/
void execute_commands (char** token, int* tp) {
	
	int length_args = *tp;
	
	if ( length_args == 0) {
	// do nothing
	} 
	//for Output redirection
	else if (strcmp(token[0], "OR") == 0) {
		redirecting_output(token, tp);
	} 
	//for piping
	else if (strcmp(token[0], "PP") == 0) {
		piping(token, tp);
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
	else if (strcmp(token[0], "exit") == 0) {
		exit(0);
	}else {
		launch_program(token);
	}
}
// adapted from: appendix_c
// prints the cmd output to a file with use of ->
void redirecting_output (char** token, int* tp) {
	
	int length_args = *tp;
	char* envp[] = {0};
	int pid, fd;
	int status;
	
	// call function to find arrows
	create_arrow_index(token, length_args);
	
	
	// store the args exluding arrow
	char* or_args [MAX_NUM_ARGS];
	int j;
	for ( j = 1; j < arrows_ind[0]; j++) {
		or_args [j - 1] = token[j]; 
	}
	// store file name
	char* output_file = token[arrows_ind[0] +1];
	// Referenece: Adapted code from appendix_c
	// write to file
	if ((pid = fork()) == 0) {
		fd = open(output_file, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR);
		if (fd == -1) {
			fprintf(stderr, "cannot open %s for writting\n", output_file);
			exit(1);
		}
		// instead of printing to stdout and stderr, print to fd
		dup2(fd, 1);
		dup2(fd, 2);
		execve(or_args[0], or_args, envp);
		fprintf(stderr, "OR: failed to execute proccess\n");
	}
	// wait for child
	waitpid(pid, &status, 0);
}
// For takes out of -> of token array and stores index in int array
void create_arrow_index (char** token, int length_args){
	
	// holds indexs of arrows
	
	int arrows_index = 0;
	int i;
	// Stores the indexs of where the arrows are.
	
	for (i = 0; i < length_args; i++) {
			
		if (strcmp(token[i], "->") == 0) {
			arrows_ind[arrows_index] = i;
			arrows_index++;
		}
	}
	
	if (arrows_index == 0) {
		fprintf (stderr, "\'->\' not found.\n");
	}
}
// adapted from: appendix_d
// pipes 
int piping(char** token, int* tp) {
	
	int length_args = *tp;
	p_create_arrow_index(token, length_args);
	
	char *cmd_head[MAX_INPUT_LINE];
    	char *cmd_tail[MAX_INPUT_LINE];
	
	// store the args before the arrow in head 
	int j;
	for ( j = 1; j < p_arrows_ind[0]; j++) {
		cmd_head [j - 1] = token[j]; 
	}
	int k;
	j=0;
	for (k = p_arrows_ind[0] + 1; k < length_args; k++) {
		cmd_tail [j] = token[k];
		j++;
	}
	
	/*for (k = 0; cmd_head[k]!= NULL; k++){
		
		printf("head is : %s\n", cmd_head[k]);
		printf("tail is : %s\n", cmd_tail[k]);
	}*/
	
    char *envp[] = { 0 };
    int status;
    int pid_head, pid_tail;
    int fd[2];
	
	pipe(fd);
	
	if ((pid_head = fork() == 0)) {
		
		dup2(fd[1], 1);
        close(fd[0]);
        execve(cmd_head[0], cmd_head, envp);
		fprintf(stderr, "PP: failed to execute proccess\n");
		return 1;
	}
	
	if ((pid_tail = fork()) == 0) {

        dup2(fd[0], 0);
        close(fd[1]);
        execve(cmd_tail[0], cmd_tail, envp);
		fprintf(stderr, "PP: failed to execute proccess\n");
		return 1;
    }
	
	close(fd[0]);
	close(fd[1]);
	
	// wait for child processes to finish
	waitpid(pid_head, &status, 0);
	waitpid(pid_tail, &status, 0); 
	return 0;
}

	
// re-cycled function for piping finding index of -> 
void p_create_arrow_index (char** token, int length_args){
	
	// holds indexs of arrows
	
	int arrows_index = 0;
	int i;
	// Stores the indexs of where the arrows are.
	
	for (i = 0; i < length_args; i++) {
			
		if (strcmp(token[i], "->") == 0) {
			p_arrows_ind[arrows_index] = i;
			arrows_index++;
		}
	}
	
	if (arrows_index == 0) {
		fprintf (stderr, "\'->\' not found.\n");
	}
}
