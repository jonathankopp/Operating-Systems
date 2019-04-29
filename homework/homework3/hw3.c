/* hw3.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include <semaphore.h>

void* thread_function(void *args);

int max_squares;
char** dead_end_boards;
int min_dead_end;
int dead_end_size = 0;

sem_t sem_squares, sem_boards;

typedef struct {
	int col;
	int row;
	char* board;
	int m;
	int n;
	int moves;
} knight_stats;


int countMultipleMoves(knight_stats* knight)
{
	int sum = 0;
	/* Left and Up */
	if(knight->col-2 >= 0 && knight->row-1 >= 0 && knight->board[(knight->row-1)*(knight->m+1) + (knight->col-2)] == '.')
	{
		++sum;
	}

	/* Up and Left */
	if(knight->col-1 >= 0 && knight->row-2 >= 0 && knight->board[(knight->row-2)*(knight->m+1) + (knight->col-1)] == '.')
	{
		++sum;
	}

	/* Up and Right */	
	if(knight->col+1 < knight->m && knight->row-2 >= 0 && knight->board[(knight->row-2)*(knight->m+1) + (knight->col+1)] == '.')
	{
		++sum;
	}

	/* Right and Up */	
	if(knight->col+2 < knight->m && knight->row-1 >= 0 && knight->board[(knight->row-1)*(knight->m+1) + (knight->col+2)] == '.')
	{
		++sum;
	}

	/* Right and Down */
	if(knight->col+2 < knight->m && knight->row+1 < knight->n && knight->board[(knight->row+1)*(knight->m+1) + (knight->col+2)] == '.')
	{
		++sum;
	}

	/* Down and Right */
	if(knight->col+1 < knight->m && knight->row+2 < knight->n && knight->board[(knight->row+2)*(knight->m+1) + (knight->col+1)] == '.')
	{
		++sum;
	}

	/* Down and Left */
	if(knight->col-1 >= 0 && knight->row+2 < knight->n && knight->board[(knight->row+2)*(knight->m+1) + (knight->col-1)] == '.')
	{
		++sum;
	}

	/* Left and Down */
	if(knight->col-2 >= 0 && knight->row+1 < knight->n && knight->board[(knight->row+1)*(knight->m+1) + (knight->col-2)] == '.')
	{
		++sum;
	}

	/* Return number of moves */
	return sum;
}


knight_stats* knightMove(knight_stats* knight)
{
	knight_stats* res;
	int rc;
	int moves = countMultipleMoves(knight);
	int index = 0;
	pthread_t tid[moves];
	++knight->moves;	
	#ifdef NO_PARALLEL
		knight_stats** total;
	#endif
	/* Multiple Moves */
	if(moves > 1)
	{
		#ifdef NO_PARALLEL
			total = calloc(moves, sizeof(knight_stats*));
		#endif
		printf("THREAD %ld: %d moves possible after move #%d; creating threads...\n", pthread_self(), moves, knight->moves);
	}
	/* No Moves */
	if(moves == 0)
	{
		if(knight->moves > max_squares)
		{
			sem_wait(&sem_squares);
			if(knight->moves > max_squares)
			{
				max_squares = knight->moves;
			}
			sem_post(&sem_squares);
		}
		/* Check if full knights tour */
		if(knight->moves == knight->m*knight->n)
		{
			printf("THREAD %ld: Sonny found a full knight's tour!\n", pthread_self());
		}
		else
		{
			/* Check if board should go on dead ends */
			printf("THREAD %ld: Dead end after move #%d\n", pthread_self(), knight->moves);
			if(knight->moves >= min_dead_end)
			{
				sem_wait(&sem_boards);
				if(dead_end_boards == NULL)
				{
					++dead_end_size;
					dead_end_boards = calloc(1, sizeof(char *));
					dead_end_boards[0] = calloc(strlen(knight->board)+1, sizeof(char));
					strcpy(dead_end_boards[0], knight->board);
				}
				else
				{
					++dead_end_size;
					dead_end_boards = realloc(dead_end_boards, dead_end_size*sizeof(char*));
					dead_end_boards[dead_end_size-1] = calloc(strlen(knight->board)+1, sizeof(char));
					strcpy(dead_end_boards[dead_end_size-1], knight->board);
				}
				sem_post(&sem_boards);
			}
		}
		return knight;
	}

	/* Left and Up */
	if(knight->col-2 >= 0 && knight->row-1 >= 0 && knight->board[(knight->row-1)*(knight->m+1) + (knight->col-2)] == '.')
	{
		if(moves == 1)
		{
			knight->col = knight->col-2;
			knight->row = knight->row-1;
			knight->board[(knight->row)*(knight->m+1) + (knight->col)] = 'S';
			res = knightMove(knight);
			return res;
		}
		else
		{
			knight_stats* newKnight = malloc(sizeof(knight_stats));
			newKnight->board = malloc((strlen(knight->board)+1)*sizeof(char));
			strcpy(newKnight->board,knight->board);
			newKnight->board[(knight->row-1)*(knight->m+1) + (knight->col-2)] = 'S';
			
			newKnight->col = knight->col-2;
			newKnight->row = knight->row-1;
			newKnight->m = knight->m;
			newKnight->n = knight->n;
			newKnight->moves = knight->moves;
			
			rc = pthread_create(&tid[index] , NULL, thread_function, newKnight);
			#ifdef NO_PARALLEL
				knight_stats* noParallelRes;	
				rc = pthread_join(tid[index], (void **) &noParallelRes);
				if(rc != 0)
				{
					fprintf(stderr, "Thread join failed.\n");
				}
				total[index] = noParallelRes;
				printf("THREAD %ld: Thread [%ld] joined (returned %d)\n", pthread_self(), tid[index], noParallelRes->moves);	
			#endif
			++index;
		}
	}

	/* Up and Left */
	if(knight->col-1 >= 0 && knight->row-2 >= 0 && knight->board[(knight->row-2)*(knight->m+1) + (knight->col-1)] == '.')
	{
		if(moves == 1)
		{	
			knight->col = knight->col-1;
			knight->row = knight->row-2;
			knight->board[(knight->row)*(knight->m+1) + (knight->col)] = 'S';
			res = knightMove(knight);
			return res;
		}
		else
		{
			knight_stats* newKnight = malloc(sizeof(knight_stats));
			newKnight->board = malloc((strlen(knight->board)+1)*sizeof(char));
			strcpy(newKnight->board,knight->board);
			newKnight->board[(knight->row-2)*(knight->m+1) + (knight->col-1)] = 'S';
			
			newKnight->col = knight->col-1;
			newKnight->row = knight->row-2;
			newKnight->m = knight->m;
			newKnight->n = knight->n;
			newKnight->moves = knight->moves;
			
			rc = pthread_create(&tid[index] , NULL, thread_function, newKnight);
			#ifdef NO_PARALLEL
				knight_stats* noParallelRes;	
				rc = pthread_join(tid[index], (void **) &noParallelRes);
				if(rc != 0)
				{
					fprintf(stderr, "Thread join failed.\n");
				}
				total[index] = noParallelRes;
				printf("THREAD %ld: Thread [%ld] joined (returned %d)\n", pthread_self(), tid[index], noParallelRes->moves);	
			#endif
			++index;
		}
	}

	/* Up and Right */	
	if(knight->col+1 < knight->m && knight->row-2 >= 0 && knight->board[(knight->row-2)*(knight->m+1) + (knight->col+1)] == '.')
	{
		if(moves == 1)
		{	
			knight->col = knight->col+1;
			knight->row = knight->row-2;
			knight->board[(knight->row)*(knight->m+1) + (knight->col)] = 'S';
			res = knightMove(knight);
			return res;
		}
		else
		{
			knight_stats* newKnight = malloc(sizeof(knight_stats));
			newKnight->board = malloc((strlen(knight->board)+1)*sizeof(char));
			strcpy(newKnight->board,knight->board);
			newKnight->board[(knight->row-2)*(knight->m+1) + (knight->col+1)] = 'S';
			
			newKnight->col = knight->col+1;
			newKnight->row = knight->row-2;
			newKnight->m = knight->m;
			newKnight->n = knight->n;
			newKnight->moves = knight->moves;
			
			rc = pthread_create(&tid[index] , NULL, thread_function, newKnight);
			#ifdef NO_PARALLEL
				knight_stats* noParallelRes;	
				rc = pthread_join(tid[index], (void **) &noParallelRes);
				if(rc != 0)
				{
					fprintf(stderr, "Thread join failed.\n");
				}
				total[index] = noParallelRes;
				printf("THREAD %ld: Thread [%ld] joined (returned %d)\n", pthread_self(), tid[index], noParallelRes->moves);	
			#endif
			++index;
		}
	}

	/* Right and Up */	
	if(knight->col+2 < knight->m && knight->row-1 >= 0 && knight->board[(knight->row-1)*(knight->m+1) + (knight->col+2)] == '.')
	{
		if(moves == 1)
		{
			knight->col = knight->col+2;
			knight->row = knight->row-1;
			knight->board[(knight->row)*(knight->m+1) + (knight->col)] = 'S';
			res = knightMove(knight);
			return res;
		}
		else
		{
			knight_stats* newKnight = malloc(sizeof(knight_stats));
			newKnight->board = malloc((strlen(knight->board)+1)*sizeof(char));
			strcpy(newKnight->board,knight->board);
			newKnight->board[(knight->row-1)*(knight->m+1) + (knight->col+2)] = 'S';
			
			newKnight->col = knight->col+2;
			newKnight->row = knight->row-1;
			newKnight->m = knight->m;
			newKnight->n = knight->n;
			newKnight->moves = knight->moves;
			
			rc = pthread_create(&tid[index] , NULL, thread_function, newKnight);
			#ifdef NO_PARALLEL
				knight_stats* noParallelRes;	
				rc = pthread_join(tid[index], (void **) &noParallelRes);
				if(rc != 0)
				{
					fprintf(stderr, "Thread join failed.\n");
				}
				total[index] = noParallelRes;
				printf("THREAD %ld: Thread [%ld] joined (returned %d)\n", pthread_self(), tid[index], noParallelRes->moves);	
			#endif
			++index;
		}
	}

	/* Right and Down */
	if(knight->col+2 < knight->m && knight->row+1 < knight->n && knight->board[(knight->row+1)*(knight->m+1) + (knight->col+2)] == '.')
	{
		if(moves == 1)
		{
			knight->col = knight->col+2;
			knight->row = knight->row+1;
			knight->board[(knight->row)*(knight->m+1) + (knight->col)] = 'S';
			res = knightMove(knight);
			return res;
		}
		else
		{
			knight_stats* newKnight = malloc(sizeof(knight_stats));
			newKnight->board = malloc((strlen(knight->board)+1)*sizeof(char));
			strcpy(newKnight->board,knight->board);
			newKnight->board[(knight->row+1)*(knight->m+1) + (knight->col+2)] = 'S';
			
			newKnight->col = knight->col+2;
			newKnight->row = knight->row+1;
			newKnight->m = knight->m;
			newKnight->n = knight->n;
			newKnight->moves = knight->moves;
			
			rc = pthread_create(&tid[index] , NULL, thread_function, newKnight);
			#ifdef NO_PARALLEL
				knight_stats* noParallelRes;	
				rc = pthread_join(tid[index], (void **) &noParallelRes);
				if(rc != 0)
				{
					fprintf(stderr, "Thread join failed.\n");
				}
				total[index] = noParallelRes;
				printf("THREAD %ld: Thread [%ld] joined (returned %d)\n", pthread_self(), tid[index], noParallelRes->moves);	
			#endif
			++index;
		}
	}

	/* Down and Right */
	if(knight->col+1 < knight->m && knight->row+2 < knight->n && knight->board[(knight->row+2)*(knight->m+1) + (knight->col+1)] == '.')
	{
		if(moves == 1)
		{
			knight->col = knight->col+1;
			knight->row = knight->row+2;
			knight->board[(knight->row)*(knight->m+1) + (knight->col)] = 'S';
			res = knightMove(knight);
			return res;
		}
		else
		{
			knight_stats* newKnight = malloc(sizeof(knight_stats));
			newKnight->board = malloc((strlen(knight->board)+1)*sizeof(char));
			strcpy(newKnight->board,knight->board);
			newKnight->board[(knight->row+2)*(knight->m+1) + (knight->col+1)] = 'S';
			
			newKnight->col = knight->col+1;
			newKnight->row = knight->row+2;
			newKnight->m = knight->m;
			newKnight->n = knight->n;
			newKnight->moves = knight->moves;
			
			rc = pthread_create(&tid[index] , NULL, thread_function, newKnight);
			#ifdef NO_PARALLEL
				knight_stats* noParallelRes;	
				rc = pthread_join(tid[index], (void **) &noParallelRes);
				if(rc != 0)
				{
					fprintf(stderr, "Thread join failed.\n");
				}
				total[index] = noParallelRes;
				printf("THREAD %ld: Thread [%ld] joined (returned %d)\n", pthread_self(), tid[index], noParallelRes->moves);	
			#endif
			++index;
		}
	}

	/* Down and Left */
	if(knight->col-1 >= 0 && knight->row+2 < knight->n && knight->board[(knight->row+2)*(knight->m+1) + (knight->col-1)] == '.')
	{
		if(moves == 1)
		{
			knight->col = knight->col-1;
			knight->row = knight->row+2;
			
			knight->board[(knight->row)*(knight->m+1) + (knight->col)] = 'S';
			res = knightMove(knight);
			return res;
		}
		else
		{
			knight_stats* newKnight = malloc(sizeof(knight_stats));
			newKnight->board = malloc((strlen(knight->board)+1)*sizeof(char));
			strcpy(newKnight->board,knight->board);
			newKnight->board[(knight->row+2)*(knight->m+1) + (knight->col-1)] = 'S';
			
			newKnight->col = knight->col-1;
			newKnight->row = knight->row+2;
			newKnight->m = knight->m;
			newKnight->n = knight->n;
			newKnight->moves = knight->moves;
			
			rc = pthread_create(&tid[index] , NULL, thread_function, newKnight);
			#ifdef NO_PARALLEL
				knight_stats* noParallelRes;	
				rc = pthread_join(tid[index], (void **) &noParallelRes);
				if(rc != 0)
				{
					fprintf(stderr, "Thread join failed.\n");
				}
				total[index] = noParallelRes;
				printf("THREAD %ld: Thread [%ld] joined (returned %d)\n", pthread_self(), tid[index], noParallelRes->moves);	
			#endif
			++index;
		}
	}

	/* Left and Down */
	if(knight->col-2 >= 0 && knight->row+1 < knight->n && knight->board[(knight->row+1)*(knight->m+1) + (knight->col-2)] == '.')
	{
		if(moves == 1)
		{
			knight->col = knight->col-2;
			knight->row = knight->row+1;

			knight->board[(knight->row)*(knight->m+1) + (knight->col)] = 'S';
			res = knightMove(knight);
			return res;
		}
		else
		{
			knight_stats* newKnight = malloc(sizeof(knight_stats));
			newKnight->board = malloc((strlen(knight->board)+1)*sizeof(char));
			strcpy(newKnight->board,knight->board);
			newKnight->board[(knight->row+1)*(knight->m+1) + (knight->col-2)] = 'S';
			
			newKnight->col = knight->col-2;
			newKnight->row = knight->row+1;
			newKnight->m = knight->m;
			newKnight->n = knight->n;
			newKnight->moves = knight->moves;
			
			rc = pthread_create(&tid[index] , NULL, thread_function, newKnight);
			#ifdef NO_PARALLEL
				knight_stats* noParallelRes;	
				rc = pthread_join(tid[index], (void **) &noParallelRes);
				if(rc != 0)
				{
					fprintf(stderr, "Thread join failed.\n");
				}
				total[index] = noParallelRes;
				printf("THREAD %ld: Thread [%ld] joined (returned %d)\n", pthread_self(), tid[index], noParallelRes->moves);	
			#endif
			++index;
		}
	}


	/* Wait for threads to finish */
	#ifndef NO_PARALLEL
		if(moves>1)
		{
			for(int i=0; i<moves; ++i)
			{
				knight_stats* res;
				rc = pthread_join(tid[i], (void **) &res);
				if(rc != 0)
				{
					fprintf(stderr, "Thread join failed.\n");
				}
				if(res->moves > knight->moves)
				{
					knight->moves = res->moves;
				}
				printf("THREAD %ld: Thread [%ld] joined (returned %d)\n", pthread_self(), tid[i], res->moves);	
				free(res->board);
				free(res);
			}
		}
	#endif

	#ifdef NO_PARALLEL
		for(int i=0; i<moves; ++i)
		{
			if(total[i]->moves > knight->moves)
			{
				knight->moves = total[i]->moves;
			}
			free(total[i]->board);
			free(total[i]);
		}
		free(total);
	#endif
	
	return knight;
}


void* thread_function(void *args)
{
	
	knight_stats* knight = (knight_stats*) args;
	knight = knightMove(knight);
	return knight;

}

int main(int argc, char** argv)
{
	/*
			Command Line Argument Parsing
	*/

	setvbuf(stdout, NULL, _IONBF, 0);

	if(argc < 3)
	{
		fprintf(stderr, "ERROR: Invalid argument(s)\n");
		fprintf(stderr, "USAGE: a.out <m> <n> [<x>]\n");
		return EXIT_FAILURE;
	}

	for(int i=0; i<strlen(argv[1]); ++i)
	{
		if(!isdigit(argv[1][i]))
		{
			fprintf(stderr, "ERROR: Invalid argument(s)\n");
			fprintf(stderr, "USAGE: a.out <m> <n> [<x>]\n");
			return EXIT_FAILURE;
		}
	}

	for(int i=0; i<strlen(argv[2]); ++i)
	{
		if(!isdigit(argv[2][i]))
		{
			fprintf(stderr, "ERROR: Invalid argument(s)\n");
			fprintf(stderr, "USAGE: a.out <m> <n> [<x>]\n");
			return EXIT_FAILURE;
		}
	}

	int m = atoi(argv[2]);
	int n = atoi(argv[1]);
	
	if(m <= 2 || n <= 2)
	{
		fprintf(stderr, "ERROR: Invalid argument(s)\n");
		fprintf(stderr, "USAGE: a.out <m> <n> [<x>]\n");
		return EXIT_FAILURE;
	}

	int x = 0;
	if(argc > 3)
	{
		for(int i=0; i<strlen(argv[3]); ++i)
		{
			if(!isdigit(argv[3][i]))
			{
				fprintf(stderr, "ERROR: Invalid argument(s)\n");
				fprintf(stderr, "USAGE: a.out <m> <n> [<x>]\n");
				return EXIT_FAILURE;
			}
		}
		x = atoi(argv[3]);
		if(x < 0 || x > m*n)
		{
			fprintf(stderr, "ERROR: Invalid argument(s)\n");
			fprintf(stderr, "USAGE: a.out <m> <n> [<x>]\n");
			return EXIT_FAILURE;
		}
	}
	min_dead_end = x;
	if(sem_init(&sem_boards, 0, 1)==-1)
	{
		fprintf(stderr, "sem_init() failed.\n");
		return EXIT_FAILURE;
	}

	if(sem_init(&sem_squares, 0, 1)==-1)
	{
		fprintf(stderr, "sem_init() failed.\n");
		return EXIT_FAILURE;
	}
	/*
			Program start
	*/

	/* Setting up initial board */
	/*
			if m = 3 (col) and n = 5 (row)

			...\n ...\n ...\n ...\n ...\0
			012 3 456 7 891 1 111 1 111 1
							      0 1 234 5 678 9
			or

			...
			...
			...
			...
			...
	*/
	printf("THREAD %ld: Solving Sonny's knight's tour problem for a %dx%d board\n", pthread_self(), n, m);
	char* board = malloc(((m+1)*n)*sizeof(char));
	for(int i=0; i<(m+1)*n; ++i)
	{
		if(i==1)
		{
			*(board) = 'S';
		}
		if(i % (m+1) == m)
		{
			*(board+i) = '\n';
		}
		else
		{
			*(board+i) = '.';
		}
	}
	board[((m+1)*n)-1] = '\0';

	knight_stats* args = malloc(sizeof(knight_stats));
	args->col = 0;
	args->row = 0;
	args->board = board;
	args->m = m;
	args->n = n;
	args->moves = 0;
	/* Start the board */
	knightMove(args);


	/* Final Outputs */
	printf("THREAD %ld: Best solution(s) found visit %d squares (out of %d)\n", pthread_self(), max_squares, m*n);
	/* Free Dynamic Memory */
	free(args->board);
	free(args);

	printf("THREAD %ld: Dead end boards:\n", pthread_self());
	for(int i=0; i<dead_end_size; ++i)
	{
		for(int j=0; j<n; ++j)
		{
			if(j==0)
			{
				printf("THREAD %ld: > ", pthread_self());
			}
			else
			{
				printf("THREAD %ld:   ", pthread_self());
			}
			for(int k=0; k<m; ++k)
			{
				putc(dead_end_boards[i][(m+1)*j+k], stdout);
			}
			printf("\n");
		}
		free(dead_end_boards[i]);
	}
	free(dead_end_boards);
	return EXIT_SUCCESS;
}
