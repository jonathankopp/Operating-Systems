#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
void * bitmoji( void * arg )
{
	int * s = (int *)arg;
	printf( "%ld lucky %d\n", pthread_self(), *s );
	return NULL;
}

int main()
{
	pthread_t tid1, tid2;
	int x = 7;
	int rc = pthread_create( &tid1, NULL, bitmoji, &x );
	rc = pthread_create( &tid2, NULL, bitmoji, &x );
	x = 13;
	rc = pthread_join( tid1, NULL );
	printf( "%d unlucky %d\n", rc, x );
	rc = pthread_join( tid2, NULL );
	return EXIT_SUCCESS;
}
