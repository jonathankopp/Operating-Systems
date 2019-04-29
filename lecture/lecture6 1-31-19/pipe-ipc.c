/* pipe-with-fork.c */

/* TO DO: identify (and draw a diagram the shows) all possible
		outputs [GOOD EXAM QUESTION] */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
	int p[2];  /* array to hold the two pipe (file) descriptors:
									p[0] is the read end of the pipe
									p[1] is the write end of the pipe */

	int rc = pipe(p); /* the input argument p will be filled in
 												with the assigned descriptors */
	if(rc == -1)
	{
		perror("pipe() failed"); /* too many descriptors opened */
		return EXIT_FAILURE;
	}

	/* fd table:

			0: stdin
			1: stdout
			2: stderr													  +--------+
			3: p[0]  <========= READ ========== | Buffer | think of this buffer as
			4: p[1]  ========== WRITE ========> | Buffer |	a temporary hidden tansient file
																					+--------+
	*/

	printf("Created pipe: p[0] is %d and p[1] is %d.\n", p[0], p[1]);
	
	
	pid_t pid = fork();  /* fork() will copy the fd table to the child */

	if(pid == -1)
	{
		perror("fork() failed.\n");
		return EXIT_FAILURE;
	}

	/* fd table:   [each process has its own fd table]
			[PARENT]																																										[CHILD]
			0: stdin																																										0: stdin
			1: stdout																																										1: stdout
			2: stderr													  +--------+																							2: stderr
			3: p[0]  <========= READ ========== | Buffer | think of this buffer as					 ===READ==>	3: p[0]
			4: p[1]  ========== WRITE ========> | Buffer |	a temporary hidden tansient file <=WRITE===	4: p[1]
																					+--------+
	*/

	/* fd table:   [each process has its own fd table]

			*** after we call close() in the child and parent below ***

			[PARENT]																																										[CHILD]
			0: stdin																																										0: stdin
			1: stdout																																										1: stdout
			2: stderr													  +--------+																							2: stderr
			3: p[0]  <========= READ ========== | Buffer | think of this buffer as					 						3:
			4:																	| Buffer |	a temporary hidden tansient file <=WRITE===	4: p[1]
																					+--------+
	*/

	if(pid == 0) /* CHILD */
	{
		close(p[0]); /* close the read end of the pipe in the child process */
		/* write data to the pipe */
		int bytes_written = write(p[1], "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
		printf("CHILD: wrote %d bytes to the pipe\n", bytes_written); /* There won't be a '\0' added in the buffer */
		
		/* read data from the pipe */
		char buffer[80];
		int bytes_read = read(p[0], buffer, 10);
		buffer[bytes_read] = '\0'; /* Assuming data is string data */
		printf("CHILD: read %d bytes: \"%s\".\n", bytes_read, buffer); 
		
		close(p[1]);
	}
	else /* PARENT */
	{
		close(p[1]); /* close the write end of the pipe in the parent process */
		/* read data from the pipe */
		char buffer[80];
		int bytes_read = read(p[0], buffer, 10);
		buffer[bytes_read] = '\0'; /* Assuming data is string data */
		printf("PARENT: read %d bytes: \"%s\".\n", bytes_read, buffer); 
		
		bytes_read = read(p[0], buffer, 10);
		buffer[bytes_read] = '\0'; /* Assuming data is string data */
		printf("PARENT: read %d bytes: \"%s\".\n", bytes_read, buffer); 

		bytes_read = read(p[0], buffer, 10); /* The buffer will read up to 10 bytes
																							if less (NOT INCLUDING 0) it's OK*/
		buffer[bytes_read] = '\0'; /* Assuming data is string data */
		printf("PARENT: read %d bytes: \"%s\".\n", bytes_read, buffer); 	
		close(p[0]);
	}

	/* fd table:   [each process has its own fd table]

			*** after the child process terminates ***

			[PARENT]																			
			0: stdin																			
			1: stdout																			
			2: stderr													  +--------+
			3: p[0]  <========= READ ========== | Buffer |	read() will return 0 if the 
			4:																	| Buffer |		pipe has no active writers
																					+--------+
																											read() will return -1 if there
																												is an error
	*/								

#if 0
 	/* THIS WILL CAUSE THE PROCESS TO HANG */
	bytes_read = read(p[0], buffer, 10); /* BLOCK indefinitely */
	buffer[bytes_read] = '\0'; /* Assuming data is string data */
	printf("Read %d bytes: \"%s\".\n", bytes_read, buffer); 
#endif

	return EXIT_SUCCESS;
}
