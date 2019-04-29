/* hw2.c */

/**
	* Author: Shayne F. Preston
	* Email: prests@rpi.edu
	*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/* For waitpid() */
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>

#define MAX 1024
#define MAXCOMMAND 64

/* found this simple parser while learning about execvp()
	http://www.csl.mtu.edu/cs4411.ck/www/NOTES/process/fork/exec.html */
int parse(char *str, char **command)
{
	int size = 0;
	while(*str != '\0')
	{
		while(*str == ' ' || *str == '\n' || *str == '\t')
		{
			*str++ = '\0';
		}
		++size;
		*command++ = str;
		while(*str != '\0' && *str != ' ' && *str != '\t' && *str != '\n')
		{
			str++;
		}
		*command = '\0';
	}
	#ifdef DEBUG_MODE
		printf("size: %d.\n", size);
	#endif
	return size;
}

char *findCommand(char *command)
{
	char *correctPath = NULL;
	char *path = getenv("MYPATH");
	char *globalPath = malloc(256);
	if(path == NULL)
	{
		strcpy(globalPath,"/bin#.");
	}
	else
	{
		strcpy(globalPath,path);
	}
	struct stat statbuf;
	if(globalPath == NULL)
	{
		fprintf(stderr, "No global path.\n");
		return NULL;
	}

	/* Parse through $MYPATH with # as deliminator */
	char *strtokPath = strtok(globalPath, "#");
	while(strtokPath != NULL)
	{
		int strtoklen = strlen(strtokPath);
		int commandlen = strlen(command);
		correctPath = malloc(commandlen + strtoklen + 2);
		if(correctPath == NULL)
		{
			fprintf(stderr, "malloc failed.\n");
			return NULL;
		}

		/* Create path */
		strncpy(correctPath, strtokPath, strtoklen);
		strncpy(correctPath + strtoklen, "/", 1);
		strncpy(correctPath + strtoklen + 1, command, commandlen);
		correctPath[commandlen+ strtoklen + 1] = '\0';
		#ifdef DEBUG_MODE
			printf("look in: %s.\n", correctPath);
		#endif

		/* Check if valid path */

		/* failed */
		if(lstat(correctPath, &statbuf)==-1)
		{
			if(errno != ENOENT)
			{
				fprintf(stderr, "lstat failed.\n");
				return NULL;
			}
		}
		else
		{
			/* path valid */
			if(statbuf.st_mode & S_IXOTH)
			{
				#ifdef DEBUG_MODE
					printf("found command %s.\n", command);
				#endif
				free(globalPath);
				return correctPath;
			}
		}

		/* Try next option */
		free(correctPath);
		strtokPath = strtok(NULL, "#");
	}

	/* No path found */
	fprintf(stderr, "ERROR: command \"%s\" not found\n", command);
	return NULL;
}

int main(int argc, char **argv)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	char *path = malloc(MAX); //Current Path
	if(path == NULL)
	{
		fprintf(stderr, "malloc failed.\n");
		return EXIT_FAILURE;
	}

	char *str = malloc(MAX); //User input

	/* Error checking malloc */
	if(str == NULL)
	{
		fprintf(stderr, "ERROR: malloc failed for variable buffer.\n");
		return EXIT_FAILURE;
	}

	int parseSize; //size of command array
	char **command; //command array
	int p[2]; //pipe
	char *commandPath;
	char *pipeCommandPath;
	int isPipe = 0;
	pid_t background1;
	pid_t background2;
	while(1)
	{
		usleep(1000);
		command = calloc(64, sizeof(char *)); //allocate 64 elements worth of commands
		if(command == NULL)
		{
			fprintf(stderr,"calloc failed.\n");
			return EXIT_FAILURE;
		}

		/* Print current directory */
		getcwd(path, MAX);
		if(path == NULL)
		{
			fprintf(stderr, "ERROR: getcwd() failed for variable path.\n");
			return EXIT_FAILURE;
		}
		printf("%s$ ", path);
		
		int status;
		int status2;
		pid_t backgroundChild = waitpid(background1, &status, WNOHANG);
		pid_t backgroundChild2 = waitpid(background2, &status2, WNOHANG);

		if(backgroundChild > 0)
		{
			printf("[process %d terminated with exit status %d]\n", backgroundChild, status);
		}

		if(backgroundChild2 > 0)
		{
			printf("[process %d terminated with exit status %d]\n", backgroundChild2, status2);
		}
		memset(str, 0, MAX);
		/* Take in input from command line through stdin */
		fgets(str, MAX, stdin);
		str[strcspn(str,"\n")] = '\0';
		
		if(strcmp(str,"")==0)
		{
			continue;
		}

		#ifdef DEBUG_MODE
			printf("%s.\n", str);
		#endif
		/* Check for exit (termination) */
		if(strcmp(str, "exit") == 0)
		{
			printf("bye\n");
			break;
		}
		
		/* Get the size of new array and trim down extra space */
		parseSize = parse(str,command);
		
		#ifdef DEBUG_MODE

			printf("CONTENTS\n");
			for(int i=0; i<parseSize; ++i)
			{
				if(command[i] != NULL)
				{
					printf("%s\n", command[i]);
				}
			}
		#endif


		/* cd command */
		if(strcmp(command[0],"cd") ==0)
		{
			if(parseSize == 1) /* cd to $HOME */
			{
				if(chdir(getenv("HOME")) == -1)
				{
					fprintf(stderr, "ERROR: execution to $HOME failed.\n");
					return EXIT_FAILURE;
				}
			}
			else if(chdir(command[1]) == -1) /* cd to another directory */
			{
				fprintf(stderr, "ERROR: execution failed.\n");
				return EXIT_FAILURE;
			}

			/* free command */
			free(command);
			continue;
		}
		
		/* Search for pipe */
		isPipe = 0;
		for(int i=0; i<parseSize; ++i)
		{
			if(strcmp(command[i],"|") == 0)
			{
				#ifdef DEBUG_MODE
					printf("found a pipe!\n");
				#endif
				isPipe = i;
			}
		}

		commandPath = findCommand(command[0]);
		if(commandPath == NULL)
		{
			free(command);
			continue;
		}
		if(isPipe > 0)
		{
			int rc = pipe(p);

			if(rc == -1)
			{
				perror("pipe() failed.\n");
				return EXIT_FAILURE;
			}

			pipeCommandPath = findCommand(command[isPipe+1]);
			if(pipeCommandPath == NULL)
			{
				free(command);
				continue;
			}
		}

		pid_t pid;
		pid = fork();
		if(pid == -1) /* Check for successful fork */
		{
			perror("ERROR: fork() failed.\n");
			return EXIT_FAILURE;
		}

		if(pid == 0) /* CHILD */
		{
			if(isPipe == 0) /* Non-piped process */
			{
				/* chop of & in child process if there */
				if(strcmp(command[parseSize-1],"&") == 0)
				{
					--parseSize;
					command[parseSize] = '\0';
					//command = realloc(command, parseSize*sizeof(char *));
					if(command == NULL)
					{
						fprintf(stderr, "ERROR: realloc failed.\n");
						exit(EXIT_FAILURE);
					}

				}

				/* Error checking for execvp() */
				if(execv(commandPath, command) < 0)
				{
					fprintf(stderr, "ERROR: execution failed.\n");
					exit(EXIT_FAILURE);
				}
			}
			else /* Piped process 1 */
			{
				/* Close read descriptor */
				close(p[0]);

				/* Set stdout to be write of pipe */
				int rc = dup2(p[1],1);
				if(rc == -1)
				{
					perror("dup2() failed.\n");
					exit(EXIT_FAILURE);
				}

				/* PARSE FIRST HALF OF COMMAND HERE */
				memset(command+isPipe, 0, 64-isPipe);
				
				commandPath = findCommand(*command);

				close(p[1]);

				/* Execute command */	
				if(execv(commandPath, command) < 0)
				{
					fprintf(stderr, "ERROR: execution failed.\n");
					exit(EXIT_FAILURE);
				}	
			}
		}
		else /* PARENT */
		{
			if(isPipe > 0)
			{
				if(strcmp(command[parseSize-1],"&") != 0)
				{
					pid_t child_pid = waitpid(pid, &status, 0);
					if(child_pid == -1)
					{
						perror("wait() 1 failed.\n");
						return EXIT_FAILURE;
					}
				
					/* Error checking */
					if(WIFSIGNALED(status))
					{
						printf("abnormal exit.\n");
					}
				}
				else
				{
					background1 = pid;
				}
				pid_t pid;
				pid = fork();

				if(pid == -1) /* Check for successful fork */
				{
					perror("ERROR: fork() failed.\n");
					return EXIT_FAILURE;
				}
				if(pid == 0) /* Process 2 */
				{
					/* Close write descriptor */
					close(p[1]);

					/* Set stdin to be read of pipe */
					int rc = dup2(p[0],0);
					if(rc == -1)
					{
						perror("dup2() failed.\n");
						return EXIT_FAILURE;
					}
					
					/* PARSE SECOND HALF OF COMMAND HERE */
					char **secondCommand = command + (isPipe + 1);
					
					parseSize -= (isPipe+1);
					for(int i=0; i<parseSize; ++i)
					{
						if(strcmp(secondCommand[i],"&")==0)
						{
							secondCommand[i] = '\0';
						}
					}

					if(secondCommand == NULL)
					{
						fprintf(stderr, "reassignment failed.\n");
						return EXIT_FAILURE;
					}
					
					char *commandPath = findCommand(*secondCommand);
				
					close(p[0]);
					/* Execute command */
					if(execv(commandPath, secondCommand) < 0)
					{
						fprintf(stderr, "ERROR: execution failed.\n");
						return EXIT_FAILURE;
					}
				}
				else /* PARENT PIPED */
				{
					close(p[0]);
					close(p[1]);
					if(strcmp(command[parseSize-1],"&") != 0)
					{
						background2 = pid;
						pid_t child_pid = waitpid(pid, &status, 0);
						if(child_pid == -1)
						{
							perror("wait() 3 failed.\n");
							return EXIT_FAILURE;
						}
					}
					else
					{
						printf("[running background process \"%s\"]\n", command[0]);
						printf("[running background process \"%s\"]\n", command[isPipe+1]);;
					}
					free(command);
					free(commandPath);
					free(pipeCommandPath);
				}
			}
			else /* PARENT NOT PIPED */
			{
				if(strcmp(command[parseSize-1],"&") != 0)
				{
					
					if(isPipe > 0)
					{
						close(p[0]);
						close(p[1]);
					}
					
					/* Wait for child process to end */
					
					pid_t child_pid = waitpid(pid, &status, 0);
					if(child_pid == -1)
					{
						perror("wait() 2 failed.\n");
						return EXIT_FAILURE;
					}
					
					/* Error checking */
					
					if(WIFSIGNALED(status))
					{
						printf("abnormal exit.\n");
					}
					
				}
				else
				{
					background1 = pid;
					printf("[running background process \"%s\"]\n", command[0]);
				}
				free(command);
				free(commandPath);
			}
		}
	}
	/* Free all dynamically allocated memory */
	for(int i=0; i<64; ++i)
	{
		free(command[i]);
	}
	free(path);
	free(command);
	free(str);

	return EXIT_SUCCESS;
}
