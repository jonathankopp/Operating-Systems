#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
int g = -9;

void* lucky(void *arg)
{
	int * q = (int*)arg;
	int rc = pthread_detach(pthread_self());

	printf("%u lucky %d-%d\n",(unsigned int)pthread_self(), rc, *q);
	return NULL;
}

void * charms(void *arg)
{
	int *q = (int *)arg;
	int rc = pthread_detach(pthread_self());
	printf("%u charms %d-%d\n", (unsigned int)pthread_self(), g, *q);
	if(g > rc)
	{
		*q = 12;
	}
	return NULL;
}

int main()
{
	pthread_t tid;
	int rc = -4, x = 4;

	printf("%u-%d-%d\n", (unsigned int) pthread_self(), rc, x);
	rc = pthread_create(&tid, NULL, lucky, &x);
	rc = pthread_create(&tid, NULL, charms, &rc);
	printf("%u-%d-%d\n", (unsigned int)pthread_self(), rc, x);
	rc = pthread_create(&tid, NULL, lucky, &x);
	printf("%u-%d-%d\n",(unsigned int)pthread_self(), rc, x);

	sleep(5);
	return 1;
}
