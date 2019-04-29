#include <stdlib.h>


int main()
{
	fork();
	if(fork())
	{
		fork();
	}
	printf("test\n");

	return EXIT_SUCCESS;
}
