/* fd-open-read.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h> /* String related strlen(), strcpy(), ect. */

/* File related */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main()
{
	char *name = "testfile.txt";

	int fd = open(name, O_RDONLY);

	if(fd == -1)
	{
		perror("open() failed");
		return EXIT_FAILURE;
	}

	printf("fd is %d\n", fd); /* this should be fd 3 */

	int j = 5;

	while(j>0)
	{
		/* TO DO: try changing this to int buffer[20] */

		char buffer[21]; /* (static) allocate 1 wxtra byte for '\0' */
		int rc = read(fd, buffer, 20);

		if(rc == -1)
		{
			perror("read() failed");
			return EXIT_FAILURE;
		}

		buffer[rc] = '\0'; /* assume the input data is a string */
		printf("read %d bytes: \"%s\"\n", rc, buffer);

		--j;
	}

	close(fd);

	return EXIT_SUCCESS;
}
