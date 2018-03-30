/*
 ============================================================================
 Name        : main.c
 Author      : Gabriel Barros
 Version     :
 Copyright   : 
 Description : Mini-Shell in C programming language
 ============================================================================
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>

#define ARG_BUF_SIZE 1024
#define TRUE 1
#define FALSE 0


int main(int argc, char *argv[])
{


	if(argc == 1)
	{ // interactive mode

		do
		{ /* for each prompt */

			pid_t pid;
			int status;
			int is_background = FALSE, barrier = FALSE;
			int buffer_size = ARG_BUF_SIZE, i=0;
			char* line = calloc(sizeof(char), buffer_size);
			char** args = calloc(sizeof(char*), buffer_size);
			char* token;

			printf("miniShell> ");
			fgets(line, buffer_size, stdin);

			if(!args)
			{
				fprintf(stderr, "Cannot allocate memory.\n");
				exit(0);
			}

			token = strtok(line, " \t\r\n\a");
			while(token != NULL)
			{
				args[i] = token;
				i++;

				token = strtok(NULL, " \t\r\n\a");
			}

			if(strcmp(args[0], "quit") == 0) /* if user types quit */
				exit(0);

			if(args[i-1][strlen(args[i-1])-1] == '&')
			{
				args[i-1][strlen(args[i-1])-1] = '\0';
				is_background = TRUE;
			}
			else if(strcmp(args[0], "barrier") == 0 || strcmp(args[0], "barrier&") == 0)
			{

				args[0] = NULL;  /* clearing 'barrier' so execvp doesn't try to execute it */
				barrier = TRUE;
			}

			pid = fork(); // spawning a process

			if(pid < 0)
			{
				perror("Fork Error");
				exit(0);
			}
			else if(pid == 0)
			{ /* CHILD process */


				if(execvp(args[0], args) == -1)
				{
					perror("Execute Command");
					exit(0);
				}

			}
			else
			{ /* PARENT process */

				if(!is_background)
				{ /* if process is not in background then wait for current process to finish */

					waitpid(pid, &status, 0);
				}

				if(barrier)
				{ /* waiting for all processes to finish before displaying next prompt */

					while((pid = waitpid(-1, &status, 0)) != -1);
				}

			}

			free(line);
			free(args);

		}while(1);

	}
	else if(argc > 2)
	{
		puts("Invalid command line argument");
		exit(0);
	}
	else
	{
		puts("Batch mode");
	}


	return EXIT_SUCCESS;
}


