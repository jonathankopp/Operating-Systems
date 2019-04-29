/* fd-dup2.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
	int fd = open("testfile.txt", O_WRONLY | O_CREAT | O_TRUNC, 0660);

		/* 0660 ==> 110 110 000
								rwx rwx rwx
								^^^ ^^^ ^^^
								 |   |   |
								 |   |   no permissions for other/world
								 |   rw for group
								 rw for the file owner */
	
	if(fd == -1)
	{
		perror("open() failed");
		return EXIT_FAILURE;
	}

	printf("fd is %d\n", fd);  /* 3 */

	/* fd table:
			
			0: stdin
			1: stdout
			2: stderr
			3: testfile.txt O_WRONLY */
	
	int rc = dup2(3,1); /* copy fd 3 into fd 1 */

	/* fd table:
		 
		 0: stdin
		 1: testfile.txt O_WRONLY
		 2: stderr
		 3: testfile.txt O_WRONLY */

	
	printf("ABCDEFGHIJKLMNOPQRSTUVWXYZ\n");

	close(fd);
	close(1);
	
	return EXIT_SUCCESS;
}
