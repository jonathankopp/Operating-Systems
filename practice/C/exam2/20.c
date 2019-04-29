#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BITMOJI 8

int main()
{
	char buffer[ BITMOJI + 1 ];
	int sd = socket( PF_INET, SOCK_STREAM, 0 );
	struct sockaddr_in server;
	server.sin_family = PF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	unsigned short port = 8888;
	server.sin_port = htons( port );
	bind( sd, (struct sockaddr *)&server, sizeof( server ) );
	listen( sd, 5 );
	printf( "SERVER: Listener bound to port %d\n", port );
	
	struct sockaddr_in client;
	int fromlen = sizeof( client );

	int clients = BITMOJI;

	while ( clients > 0 )
	{
		printf( "SERVER: Blocked on accept()\n" );
		int newsd = accept( sd, (struct sockaddr *)&client, (socklen_t *)&fromlen );
		
		while ( 1 )
		{
			printf( "SERVER: Blocked on recv()\n" );
			int n = recv( newsd, buffer, BITMOJI, 0 );
			buffer[n] = '\0';
			printf( "SERVER: Rcvd [%s]\n", buffer );
		}
		
		close( newsd );
		clients--;
	}
	
	printf( "SERVER: done\n" );
	return EXIT_SUCCESS;
}
