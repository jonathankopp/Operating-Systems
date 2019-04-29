/* realloc.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
	/* similar: int z[100]; */
	int *z = calloc( 100, sizeof(int));
	
	if(z==NULL)
	{
		fprinf(stderr, "ERROR: calloc()
	}

	return EXIT_SUCCESS:
}
