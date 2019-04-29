/* udp-server.c */

/* To test this server, you can use the following
		command-line netcat tool:


*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define MAXBUFFER 8192

int main()
{
	int sd; /* socket descriptor -- this is actually in the fd table! */
	struct sockaddr_in server;
	int length;

	/* create the socket (endpoint) on the server side */
	sd = socket(AF_INET, SOCK_DGRAM, 0);
										/* ^^^^^^^^^^
											this will set this socket up to use UDP */
	if(sd == -1)
	{
		perror("socket() failed");
		return EXIT_FAILURE;
	}

	server.sin_family = AF_INET; /* IPv4 */
	server.sin_addr.s_addr = htonl(INADDR_ANY);

	/* specify the port number for the server */
	server.sin_port = htons(0); /* a 0 here means let the kernel assign
																	us a port number to listen on */
	
	if(bind(sd, (struct sockaddr *) &server, sizeof(server)) < 0)
	{
		perror("bind() failed");
		return EXIT_FAILURE;
	}

	length = sizeof(server);

	/* call getsockname() to obtain the port number that was just assigned */
	if(getsockname(sd, (struct sockaddr *)))
	{
	}
}
