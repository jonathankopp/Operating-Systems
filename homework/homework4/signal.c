/* signals.c */

/* suggest not to use signals on hw2 for SIGCHLD !!! */

/* check out: "man 7 signal" */
/* check out: "man 2 signal" */
/* check out: "man sigaction" */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

void signal_handler( int sig )
{
  if ( sig == SIGINT )
  {
    printf( "Stop hitting CTRL-C and answer the question!\n" );
  }
  else if ( sig == SIGUSR1 )
  {
    printf( "Rcvd SIGUSR1 -- reloading the config file (pretend).....\n" );
  }
}

int main()
{
  signal( SIGINT, SIG_IGN );   /* ignore SIGINT */
  signal( SIGINT, SIG_DFL );   /* restore to the default behavior */

  /* redefine SIGINT to call signal_handler() */
  signal( SIGINT, signal_handler );
  signal( SIGUSR1, signal_handler );

  char name[100];
  printf( "Enter your name: " );
  scanf( "%s", name );
  printf( "Hi, %s\n", name );

  return EXIT_SUCCESS;
}
