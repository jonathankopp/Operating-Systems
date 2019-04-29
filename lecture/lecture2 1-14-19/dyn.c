/* dyn.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
	char ** cache;
	cache = calloc(47, sizeof(char *));

	if(cache ==  NULL)
	{
		fprintf(stderr, "ERROR: calloc() failed.\n");
		return EXIT_FAILURE;
	}


	char input[100] = "ABCD";

	printf("input[73] is '%c'\n", input[73]);

	cache[3] = calloc(strlen(input) + 1, sizeof(char));

	strcpy(cache[3], input);
	
	free(cache[3]); /* Smarter to use loop through all of the elements */
	free(cache);

	return EXIT_FAILURE;
}
