/* buffering.c */

#include <stdio.h>
#include <stdlib.h>

int main()
{
	printf("HERE0"); /* without '\n' on the end of the line the "HERE0" string is buffered, meaning we will
										 not see it during a seg fault because it's never pushed out */
	
	fflush(stdout); /* flush the stdout buffer, i.e. output everything that wouldn't be seen in terminal */

	int * xyz = NULL; /* define something to a variable don't just leave it uninitialized (int * xyz;) */

	*xyz = 1234;

	printf("xyz points to %d\n", *xyz);

	return EXIT_SUCCESS;
}
