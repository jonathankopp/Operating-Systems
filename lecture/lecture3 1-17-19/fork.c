/* fork.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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
	}
	/* could see the two lines swapped because they're running at the same time */
	
	printf("All done....exiting....\n");

	return EXIT_SUCCESS;
}
