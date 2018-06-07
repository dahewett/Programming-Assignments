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
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>


#define MAX_INPUT_LINE 80
#define MAX_NUM_ARGS 7
#define MAX_PROMPT 10


void cmd_loop(char* input, char** token, int* tp, char* t);
void tokenize (char* input, char** token, int* tp, char* t);
void launch_program (char** token);
void execute_commands (char** token, int* tp);
void read_file();
void redirecting_output(char** token, int* tp);
void create_arrow_index (char** token, int length_args);

char prompt [MAX_PROMPT];
char direct [10][10];
int arrows_ind [MAX_NUM_ARGS];

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
		// Use a char pointer to go through the 2D array
		char* dirP;
		dirP = direct[i];
		
		if( dirP[strlen(dirP) - 1] == '\n') {
			dirP[strlen(dirP) - 1] = '\0'; 
		}
		i++;
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
		//printf("child: about to start...\n");
		//printf("%s\n", token[0]);
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
	} else if (strcmp(token[0], "OR") == 0) {
		redirecting_output(token, tp);
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
		//exit(0);
	}else {
		launch_program(token);
	}
}

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
	// Referenece: Addapted code from appendix_c
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
		fprintf(stderr, "OR: failed to execute proccess/n");
	}
	// wait for child
	waitpid(pid, &status, 0);
}

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
	
	
	
	
	
	
	