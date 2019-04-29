/* fork.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* For waitpid */
#include <sys/types.h>
#include <sys/wait.h>

int main()
{
	pid_t pid; /* process id (pid) */
	
	printf("before fork()...\n");

	/* create a new (child) process */
	pid = fork();

	/* fork() will (atempt to) create a new process by
		 duplicating/copying the existing running process */
	
	if(pid == -1)
	{
		perror("fork () failed");
		return EXIT_FAILURE;
	}

	printf("after fork()...\n");

	if(pid == 0)
	{
		printf("CHILD: happy birthday!\n");
	}
	else /* pid > 0 */
	{
		//sleep(1); <-- just to prove
		printf("PARENT: my child process ID is %d\n", pid);


		int status;
		pid_t child_pid = wait( &status); /* wait() BLOCKS indefinetly */

		printf("PARENT: child process %d terminated..\n", child_pid);

		if(WIFSIGNALED(status))						/* child process was terminated due to */
		{																	/* a signal (e.g. seg fault) */
			printf("...abnormally\n");
		}
		else
		{
			int exit_status = WEXITSTATUS(status);
			printf("...successfully with exit status %d\n", exit_status);
		}

	}
	/* could see the two lines swapped because they're running at the same time */
	
	printf("All done....exiting....\n");

	return EXIT_SUCCESS;
}
