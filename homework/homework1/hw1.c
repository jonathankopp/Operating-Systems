/* hw1.c */

/**
	* Author: Shayne F. Preston
	* Email: prests@rpi.edu
	*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

/* FIle Related */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX 128

/* Checks ascii value to see if int is a special character */
int specialChecker(int c)
{
	if((c >= 33 && c <= 47) || (c >= 58 && c <= 64) || (c >= 91 &&  c <= 96) || (c >= 123))
	{
		return 1;
	
	}
	else
	{
		return 0;
	}
}
/* sum up ASCII values and (mod cacheSize) to return index of word */
int cacheIndex(char* word, size_t cacheSize)
{
	char * ptr = word;
	int sum = 0;
	for(char c=*ptr; c; c=*++ptr)
	{
		sum += (int) c;
		#ifdef DEBUG_MODE
			printf("char %c has value %d.\n", c, c);
		#endif
	}	
	#ifdef DEBUG_MODE
		printf("sum: %d.\n", sum);
		printf("sum with mod %ld = %d.\n", cacheSize, sum%(int)cacheSize);
	#endif
	return sum%cacheSize;
}

int main(int argc, char** argv)
{
	#ifdef DEBUG_MODE
		printf("argc is %d.\n", argc);
		printf("argv index: 1 is %s.\n", *(argv+1));
		printf("path is %s.\n", *(argv+2));
	#endif

	/* Checking for argument */
	if(argc < 2)
	{
		fprintf(stderr, "ERROR: invalid command arguments.\n");
		return EXIT_FAILURE;
	}
	
	/* checks to see if first command line argument is an integer */
	char * size = *(argv+1);
	for(int i=0; i<strlen(size); ++i)
	{
		#ifdef DEBUG_MODE
			printf("argv char at %d is %c.\n", i, *(size+i));
		#endif
		if(!isdigit(*(size+i)))
		{
			fprintf(stderr, "size is not interger.\n");
			return EXIT_FAILURE;
		}
	}

	/* Creating hash table with size and allocating memory */
	size_t cacheSize = atoi(*(argv+1));
	char ** hashCache;
	hashCache = calloc(cacheSize, sizeof(char *));
	
	/* Error Checking after memory allocation */
	if(hashCache == NULL)
	{
		fprintf(stderr, "ERROR: calloc failed for variable hashCache.\n");
		return EXIT_FAILURE;
	}
	
	/* Dynamically allocated word buffer to read up to 127 + '\0' character words */
	char * word;
	word = malloc(MAX);
	
	/* Error Checking after memory allocation */
	if(word == NULL)
	{
		fprintf(stderr, "ERROR: malloc failed for variable word.\n");
		return EXIT_FAILURE;
	}

	/* File pointer for second commandline argument */
	FILE *fp;

	/* Making sure proper file path is provided */
	if((fp = fopen(*(argv+2), "r")) == NULL)
	{
		perror("open file failed.\n");
		return EXIT_FAILURE;
	}

	/* character being read in as an int of asii value */
	int c;

	/* read until error or EOF */
	while(1)
	{
		/* Obtain next ascii value */
		c = fgetc(fp);

		/* Check for EOF */
		if(feof(fp))
		{
			break;
		}
		
		#ifdef DEBUG_MODE
			printf("char digit: %d is: %c.\n", c, c);
		#endif
		/* Make sure that the input is just a letter not a space, new line, or special char */
		if(c != 32 && c != 10 && !ispunct(c) && (specialChecker(c) == 0))
		{
			*(word+strlen(word)) = c;
			*(word+strlen(word)+1) = '\0';
			continue;
		}

		/* Ensure word is atleast 3 characters long */
		if(strlen(word)<3)
		{
			memset(word, 0, strlen(word));
			continue;
		}

		/* Append the NULL value to show end of word */
		*(word+strlen(word)) ='\0';

		/* Calculate index of word */
		int index = cacheIndex(word, cacheSize);
		#ifdef DEBUG_MODE
			printf("Size of word: %ld Size of index %ld.\n", sizeof(word), sizeof(*(hashCache+index)));
			printf("word: %s has index: %d.\n", word, index);
		#endif

		/* Determine if calloc or realloc */
		if(*(hashCache+index)==NULL)
		{
			/* Create memory for unassigned index spot */
			*(hashCache+index) = calloc(strlen(word)+1, sizeof(char));

			/* Error Checking after memory allocation */
			if(*(hashCache+index) == NULL)
			{
				fprintf(stderr, "ERROR: calloc failed for variable *(hacheCache+%d).\n", index);
				return EXIT_FAILURE;
			}
			
			/* Copy word into index spot */
			strcpy(*(hashCache+index), word);
			printf("Word \"%s\" ==> %d (calloc)\n", word, index);
		}
		else
		{
			/* Reallocate memory for index spot */
			*(hashCache+index) = realloc(*(hashCache+index), strlen(word)+1);
			
			/* Error Checking after memory reallocation */
			if(*(hashCache+index) == NULL)
			{
				fprintf(stderr, "ERROR: calloc failed for variable *(hacheCache+%d).\n", index);
				return EXIT_FAILURE;
			}
			
			/* Copy word into index spot */
			strcpy(*(hashCache+index), word);
			printf("Word \"%s\" ==> %d (realloc)\n", word, index);
		}

		/* Reset word buffer to be used again */
		memset(word, 0, strlen(word));
	}
	/* Close out file */
	fclose(fp); 

	/* Free Dynamic Memory */
	for(int i=0; i<cacheSize; ++i)
	{
		/* Output all non-empty entries */
		#ifdef DEBUG_MODE /* DEBUG_MODE outputs all */
			printf("value at %d: %s.\n", i, *(hashCache+i));
		#endif
		if(*(hashCache+i) != NULL)
		{
			printf("Cache index %d ==> \"%s\"\n", i, *(hashCache+i));
		}
		/* Free at index */
		free(*(hashCache+i));
	}
	/* Free pointer to array */
	free(hashCache);
	/* Free word buffer */
	free(word);
	return EXIT_SUCCESS;
}
