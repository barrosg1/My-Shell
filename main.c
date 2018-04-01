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
#include <fcntl.h>

#define ARG_BUF_SIZE 1024
#define TRUE 1
#define FALSE 0

void mini_shell_loop(int, FILE*);

int main(int argc, char *argv[])
{
    
    int batchMode = TRUE;
    
    if(argc == 1)
    { // interactive mode
        
        batchMode = FALSE;
        mini_shell_loop(batchMode, NULL);
        
    }
    else if(argc > 2)
    {
        puts("Invalid command line argument");
        exit(0);
    }
    else
    { // batch mode
        
        FILE* fp;
        char* filename = argv[1];
        
        fp = fopen(filename, "r");
        if(fp == NULL)
        {
            perror("Error opening the file");
            exit(0);
        }
        
        mini_shell_loop(batchMode, fp);
        
    }
    
    return EXIT_SUCCESS;
}


void mini_shell_loop(int batchMode, FILE* file)
{
    do
    { /* for each prompt */
        
        pid_t pid;
        int status;
        int doInBackground = FALSE, barrier = FALSE;
        int buffer_size = ARG_BUF_SIZE, i=0, argCount = 0;
        char* line = calloc(sizeof(char), buffer_size);
        char** args = calloc(sizeof(char*), buffer_size);
        char* token;
        
        while((pid = waitpid(-1, &status, WNOHANG)) > 0); /* background processes termination */
        
        if(!batchMode)
        {
            printf("miniShell> ");
            fgets(line, buffer_size, stdin); /* read command in interactive mode */
            
        }
        else
        {
            fgets(line, buffer_size, file); /* read file in batch mode */
            
            if(feof(file)) exit(0); /* if end of file is reached */
        }
        
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
            
            argCount++;
            
            token = strtok(NULL, " \t\r\n\a");
        }
        
        if(args[0] == '\0') continue;
        if(strcmp(args[0], "quit") == 0) exit(0); /* if user types quit */
        
        if(strcmp(args[0], "barrier") == 0 || strcmp(args[0], "barrier&") == 0)
        {
            args[0] = NULL;  /* clearing 'barrier' so execvp doesn't try to execute it */
            barrier = TRUE;
        }
        else if(args[i-1][strlen(args[i-1])-1] == '&')
        {
            args[i-1][strlen(args[i-1])-1] = '\0';
            doInBackground = TRUE;
        }
        
        
        
        /* ----------------------- Handling processes ----------------------- */
        
        
        pid = fork(); // spawning a process
        
        if(pid < 0)
        {
            perror("Fork Error");
            exit(0);
        }
        else if(pid == 0)
        { /* CHILD process */
            
            
            if(argCount >= 3)
            {
                if(strcmp(args[i-2], ">") == 0)
                { /* redirect output */
                    
                    args[i-2] = NULL;
                    
                    int fd = creat(args[i-1], 0644);
                    if(fd == -1)
                    {
                        perror("Redirect File");
                        exit(0);
                    }
                    dup2(fd, STDOUT_FILENO);
                    close(fd);
                }
            }
            
            if(execvp(args[0], args) == -1)
            {
                fprintf(stderr, "%s: command not found\n", args[0]);
                break;
            }
            
        }
        else
        { /* PARENT process */
            
            if(!doInBackground)
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
