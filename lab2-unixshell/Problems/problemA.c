/*
 * Matt Morgis
 * CIS 3207 - Temple University
 * Basic Unix Shell 
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <sys/wait.h>

void parse(char *userInput, int forking);
char **complie_argv(char *userInput);
void execute(char *userInput);


/*
 *
 * Main method of program.
 * 
 * Runs infinitely until the user types "exit" or ctrl-d.
 * Takes an input from the user.
 * Prints the output to stdin
 * calls basic parse method to handle the input entered.
 *
 */
int main(int argc, char const *argv[])
{
	int forking;

	while (1)
	{
		forking = 0;
		printf("MyShell> ");

		char *userInput = (char *) malloc(sizeof(char) *1024);
		char *output = fgets(userInput, 1024, stdin);

		//Handle exit situations.
		//User types 'exit' or  ctrl + d
		if (feof(stdin) || (strcmp(output, "exit\n") == 0)) 
        {
            printf("Exiting shell.\n");
            free(userInput);
            exit(0);
        }
        int length = strlen(output);

        /*
         * Handle Problem B here.
         * Check for & at the end of the command
         *
         */

        parse(output, forking);

	}

	return 0;
}

/*
 * 
 * Parses the input from the user.
 * Finds the length.
 * Changes the tailing '\n' to '\0'
 *
 * Calls execute method.
 *
 */
void parse(char *userInput, int forking)
{
	size_t length = strlen(userInput) - 1;

	if (userInput[length] == '\n')
	{
		userInput[length] = '\0';
	}

    execute(userInput);
	return;
}

/*
 *
 * Complies the argv array that will be passed to execvp.
 * Creates a temp array and copies the userInput to it.
 * Loops through the command and parameters one at a time and stores in "current" variable.
 * Adds the current variable to argv and mallocs as needed.
 *
 */
char **complie_argv(char *userInput)
{
	int i = 0;
	
	char **argv = (char**) malloc(sizeof (char*));
	char *temp = (char*) malloc(sizeof(char) * (strlen(userInput) + 1));

	strncpy(temp, userInput, strlen(userInput) + 1);

	char *current = strtok(temp, " "); //current place in 


	while (current != NULL)
	{	
		argv[i] = (char*) malloc(sizeof(char) * strlen(current) + 1); 
        strncpy(argv[i], current, strlen(current) + 1); 
        i++; 
        current = strtok(NULL, " "); 
        argv = (char**) realloc(argv, sizeof(char*) * (i+1)); // change size of argv for next iteration
	}

	argv[i] = NULL; 
    free(temp);
    return argv;

}

/*
 *
 * Called to execute a basic command.
 * Complies the argv array.
 * Passes to execvp to execute the command with parameters.
 *
 */
void execute(char *userInput)
{
    char **argv = complie_argv(userInput); 
    if (fork() == 0)
    {
        execvp(argv[0], argv);
    }
    else
    {
        int status = 0;
        wait(&status);
    }
}