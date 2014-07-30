/*
 * Matt Morgis
 * CIS 3207 - Temple University
 * Basic Unix Shell 
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
void execute_gt(char *userInput);
void execute_lt(char *userInput);
void execute_pipe(char *userInput);
char **str_split(char *a_str, const char a_delim);
char *trimwhitespace(char *str);

/*
 *
 * Main method of program.
 * 
 * Runs infinitely until the user types "exit" or ctrl-d.
 * Takes an input from the user.
 * Prints the output to stdin
 * calls basic parse method to handle the input entered.
 * 
 * Problem B: Now checks for & at end of command.
 * Sets forking and trips the & off before calling parse.
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
         */
        const char *lastTwoChars = &output[length-2];
        if (strcmp(lastTwoChars, "&\n") == 0) 
        {
            forking = 1; 
            output[length-2] = '\0'; 
        }

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
 * Problem B:
 * If forking is set, calls method to build argv
 * Otherwise, calls execute method.
 *
 * Problem C:
 * Now checks for '<' '>' and '|' in the command.
 * If one is found, calls approipate method
 * to handle them.
 *
 */
void parse(char *userInput, int forking)
{
    size_t length = strlen(userInput) - 1;

    if (userInput[length] == '\n')
    {
        userInput[length] = '\0';
    }

    if (strstr(userInput, "<") != NULL)
    {
        if (forking == 1)
        {
            if (fork() == 0)
            {
                execute_lt(userInput);
            }
        }
        else 
        {
            execute_lt(userInput);
        }
    }

    else if (strstr(userInput, ">") != NULL)
    {
        if (forking == 1)
        {
            if (fork() == 0) 
            {
                execute_gt(userInput);
            }
            else
            {
                return;
            }
        }
        else 
        {
            execute_gt(userInput);
        }
    }

    else if (strstr(userInput, "|") != NULL)
    {
        if (forking == 1)
        {
            if (fork() == 0) 
            {
                execute_pipe(userInput);
            }
            else
            {
                return;
            }
        }
        else 
        {
            execute_pipe(userInput);
        }
    }

    else
    {
        if (forking == 1)
        {
            if (fork() == 0)
            {
                char **argv = complie_argv(userInput);
                execvp(argv[0], argv);
            }
            else
            {
                return;
            }
        }
        else
        {
            execute(userInput);
        }
    }
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
 * Called to execute a basic command.
 * Complies the argv array.
 * Passes to execvp to execute the command with parameters.
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

/*
 * Called to execute the command when a '<'
 * Is passed to it.
 *
 * Breaks the user inputted command at the <.
 * Removes any white space between the args
 * Builds argv and then executes the command.
 */
void execute_lt(char *userInput) 
{
    char **inputArgs = str_split(userInput, '<'); 
    inputArgs[1] = trimwhitespace(inputArgs[1]);
    int newstdin = open(inputArgs[1], O_RDONLY);
    char **argv = complie_argv(inputArgs[0]); 
    if (fork() == 0)
    {
        dup2(newstdin, 0);
        execvp(argv[0], argv);
    }
    else
    {
        close(newstdin);
        int status = 0;
        wait(&status);
    }
    return;
}

/*
 * Called to execute the command when a '<'
 * Is passed to it.
 *
 * Breaks the user inputted command at the >.
 * Removes any white space between the args
 * Builds argv and then executes the command.
 */
void execute_gt(char *userInput)
{
    char **outputArgs = str_split(userInput, '>');
    outputArgs[1] = trimwhitespace(outputArgs[1]);
    int newstdout = open(outputArgs[1], O_WRONLY|O_CREAT,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    char **argv = complie_argv(outputArgs[0]);
    if (fork() == 0)
    {
        dup2(newstdout, 1);
        execvp(argv[0], argv);
    }
    else
    {
        close(newstdout);
        int status = 0;
        wait(&status);
    }
    return;
}

/*
 * Called to execute the command when a '|'
 * Is passed to it.
 *
 * It builds two sets of argv - left and right.
 * Then, properly handles the execution of the two
 * commands.
 */
void execute_pipe(char *userInput)
{
    char **pipeArgs = str_split(userInput, '|');
    char **left = complie_argv(pipeArgs[0]);
    char **right = complie_argv(pipeArgs[1]);

    //initialize
    int thePipe[2];
    pipe(thePipe);

    //fork the pipe
    int pipeID = fork();
    if (pipeID > 0)
    {
       
        close(thePipe[1]); // close pipe in before forking to prevent hangs
        
        //execute the right side
        int pipeID2 = fork();
        if (pipeID2 > 0)
        {
            
            close(thePipe[0]); 
            int status2 = 0;
            wait(&status2); //wait for right child to exit
        }
        else
        {
            //right side's child process
            dup2(thePipe[0], 0); 
            execvp(right[0], right);
        }
        close(thePipe[0]);

        int status = 0;
        wait(&status); //wait for left to exit
    }
    else
    {   
        //left side child process
        close(thePipe[0]); 
        dup2(thePipe[1], 1); 
        execvp(left[0], left);
    }
    return;
}

/*
 *
 * Mimincs the Java method of splitting strings.
 * Returns **array that is broken up by the 
 * Deliminator that is passed to it.
 *
 * More info can be found here: http://stackoverflow.com/questions/9210528/split-string-with-delimiters-in-c
 */
char **str_split(char *a_str, const char a_delim)
{
    char **result = 0;
    size_t count = 0;
    char *tmp = a_str;
    char *lastComma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp)
    {
        if (a_delim == *tmp)
        {
            count++;
            lastComma = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += lastComma < (a_str + strlen(a_str) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;

    result = (char**) malloc(sizeof(char*) * count);

    if (result)
    {
        size_t idx  = 0;
        char *token = strtok(a_str, delim);

        while (token)
        {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }

    return result;
}

/*
 * Basic method to trim whitespace from a C-string. 
 * More info: http://stackoverflow.com/questions/122616/how-do-i-trim-leading-trailing-whitespace-in-a-standard-way
 *
 */
char *trimwhitespace(char *str)
{
  char *end;

  // Trim leading space
  while(isspace(*str)) str++;

  if(*str == 0)  // All spaces?
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace(*end)) end--;

  // Write new null terminator
  *(end+1) = 0;

  return str;
}