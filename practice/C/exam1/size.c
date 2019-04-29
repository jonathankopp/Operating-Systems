#include <stdio.h>

int main()
{
	printf("Pointer %ld.\n", sizeof(double *));
	printf("Char %ld\n", sizeof(char));
	printf("Int %ld\n", sizeof(int));
	printf("Float %ld\n", sizeof(float));
	printf("Double %ld\n", sizeof(double));
	printf("struct %ld\n", sizeof(struct));
	return 0;
}
