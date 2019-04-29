#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

int main()
{
	int y = 0;
	int x = y;
	x += 5;
	printf("%d %d\n", y, x);
	return EXIT_SUCCESS;
}
