#include <stdio.h>

#define TEMP 0 /* Global Variable */

main()
{
	char str1[20];
	printf("TEMP is %d\n", TEMP);
	printf("Hello World\n");
	scanf("%s", str1);
	printf("scanf read %s\n", str1);

  char c = getchar();
	putchar(c);

	(1 > 0) ? printf("Of course it is you dingus!\n") : printf("All hope is lost...\n");
	/*
	if(1>0){
		printf("Of course it is you dingus!\n");
	}
	else {
		printf("All hope is lost...\n");
	}
	*/

}
