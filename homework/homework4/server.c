/* server-select.c */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/select.h>      /* <===== */
#include <signal.h>
#include <ctype.h>

#define BUFFER_SIZE 1024
#define MAX_CLIENTS 100      /* <===== */

typedef struct {
	char* username;
	int fd;
} userProfile;


userProfile **users;
int numUsers;
sem_t sem_user;

void *thread_function(void* args);
userProfile *login(char *name, int fd);
int parseInput(char *str, char **command);
char *who();
void broadcast(char *message, char *msglen, char *sender);
void sendMsg(char *message, char *msglen, char *person, char *sender);

void signal_handler(int sig)
{
	if(sig == SIGINT)
	{
		if(users != NULL)
		{
			for(int i=0; i<numUsers; ++i)
			{
				free(users[i]);
			}
			free(users);
		}
	}
	exit(0);
}


userProfile *login(char *name, int fd)
{
	if(strlen(name) > 16 || strlen(name) < 4)
	{
		printf("CHILD %ld: Sent ERROR (Invalid userid)\n", pthread_self());
		send(fd, "ERROR Invalid userid\n", 21, 0); 
		return NULL;
	}
	sem_wait(&sem_user);
	if(numUsers == 0)
	{
		users = calloc(1, sizeof(userProfile *));
		userProfile *newUser = malloc(sizeof(userProfile));
		newUser->username = malloc((strlen(name)+1)*sizeof(char));
		newUser->fd = fd;
		strcpy(newUser->username,name);
		newUser->username[strlen(name)] = '\0';		
		users[numUsers] = newUser;
		++numUsers;
		sem_post(&sem_user);
		return newUser;
	}
	else
	{
		for(int i=0; i<numUsers; ++i)
		{
			if(strcmp(users[i]->username,name) == 0)
			{
				printf("CHILD %ld: Sent ERROR (Already connected)\n", pthread_self());
				send(fd,"ERROR Already connected\n", 24, 0);
				sem_post(&sem_user);
				return NULL;
			}
		}
		users = realloc(users,(numUsers+1)*sizeof(userProfile));
		userProfile *newUser = malloc(sizeof(userProfile));
		newUser->username = malloc((strlen(name)+1)*sizeof(char));
		newUser->fd = fd;
		strcpy(newUser->username,name);
		newUser->username[strlen(name)] = '\0';
		users[numUsers] = newUser;
		++numUsers;
		for(int i=0; i<numUsers-1; ++i)
		{
			for(int j=0; j<numUsers-i-1; ++j)
			{
				if(strcmp(users[j]->username,users[j+1]->username) > 0)
				{
					userProfile *temp = users[j];
					users[j] = users[j+1];
					users[j+1] = temp;
				}
			}
		}
		sem_post(&sem_user);
		return newUser;
	}
}

void logout(char *name)
{	
	sem_wait(&sem_user);
	for(int i=0; i<numUsers; ++i)
	{
		if(strcmp(users[i]->username, name) == 0)
		{
			send(users[i]->fd, "OK!\n", 4, 0);
			free(users[i]->username);
			free(users[i]);
			for(int j=i; j<numUsers; ++j)
			{
				if(j != numUsers-1)
				{
					users[j] = users[j+1];
				}
			}
			users = realloc(users, (numUsers-1)*sizeof(userProfile));
			--numUsers;
			break;
		}
	}
	sem_post(&sem_user);
}

char *who()
{
	sem_wait(&sem_user);
	char *userList = calloc(BUFFER_SIZE, sizeof(char));
	strcat(userList,"OK!\n");
	for(int i=0; i<numUsers; ++i)
	{
		strcat(userList,users[i]->username);
		strcat(userList,"\n");
	}
	strcat(userList,"\0");
	sem_post(&sem_user);
	return userList;
}

void broadcast(char *message, char *msglen, char *sender)
{
	for(int i=0; i<numUsers; ++i)
	{
		/*
		if(message[strlen(message)-1] == '\n')
		{
			message[strlen(message)-1] = '\0';
		}
		*/
		char *forNewClient = calloc(9+strlen(sender)+strlen(msglen)+strlen(message), sizeof(char));
		strcat(forNewClient, "FROM ");
		strcat(forNewClient, sender);
		strcat(forNewClient, " ");
		strcat(forNewClient, msglen);
		strcat(forNewClient, " ");
		strcat(forNewClient, message);
		strcat(forNewClient, "\n");	
		send(users[i]->fd, forNewClient, strlen(forNewClient), 0);
		free(forNewClient);
	}	
}
/*
void sendMsg(char *message, char *msglen, char *person, char *sender)
{
	for(int i=0; i<numUsers; ++i)
	{
		if(strcmp(users[i]->username,person)==0)
		{
			//send(users[i]->fd, forNewClient, strlen(forNewClient), 0);
			//free(forNewClient);
		}
	}
}
*/
void * thread_function(void* args)
{
	int *threadSock = (int*) args;
	char buffer[BUFFER_SIZE];
	userProfile *account = NULL;
	int broadcastMessage = 0;
	int sendMessage = 0;
	char *msglenStr = "";
	int* msglen;
	char *person;
	while(1)
	{
		ssize_t msg = read(*threadSock, buffer, BUFFER_SIZE);
		if(msg == 0)
		{
			break;
		}
		if(buffer[msg-1] == '\n')
		{
			buffer[msg-1] = '\0';
		}
		else
		{
			buffer[msg] = '\0';
		}
		
		if(strcmp(buffer,"WHO") == 0 && broadcastMessage == 0 && sendMessage == 0)
		{
			printf("CHILD %ld: Rcvd WHO request\n", pthread_self());
			if(account == NULL)
			{
				send(*threadSock, "Not logged in\n", 14, 0);
				continue;
			}
			//WHO
			char *listOfUsers = who();
			send(*threadSock, listOfUsers, strlen(listOfUsers), 0);
			free(listOfUsers);
			continue;
		}

		if(strcmp(buffer,"LOGOUT") == 0 && broadcastMessage == 0 && sendMessage == 0)
		{
			printf("CHILD %ld: Rcvd LOGOUT request\n", pthread_self());
			if(account == NULL)
			{
				send(*threadSock, "Not logged in\n", 14, 0);
				continue;
			}
			//LOGOUT
			logout(account->username);
			account = NULL;
			continue;
		}

		char delimiter[] = " ";
		char *command;
		char *remainder;
		char *inputCopy = (char*) calloc(msg + 1, sizeof(char));
		strncpy(inputCopy, buffer, msg);
		command = strtok_r(inputCopy, delimiter, &remainder);
		if(strcmp(command,"LOGIN") == 0 && broadcastMessage == 0 && sendMessage == 0)
		{
			printf("CHILD %ld: Rcvd LOGIN request for userid %s\n", pthread_self(), remainder);
			if(account != NULL)
			{
				send(*threadSock, "Already logged in\n", 18, 0);
			}
			else
			{
				//LOGIN
				account = login(remainder, *threadSock);
				if(account != NULL)
				{
					send(*threadSock, "OK!\n", 4, 0);
				}
			}
		}
		else
		{
			if(strcmp(command,"SHARE") == 0 && broadcastMessage == 0 && sendMessage == 0)
			{
				if(account == NULL)
				{
					send(*threadSock, "Not logged in\n", 14, 0);
				}
				else
				{
					//SHARE
					int found = 0;
					char *sendName = strtok_r(NULL," ", &remainder);
					printf("CHILD %ld: Rcvd SHARE request\n", pthread_self());
					userProfile *receiver;
					sem_wait(&sem_user);
					for(int i=0; i<numUsers; ++i)
					{
						if(strcmp(users[i]->username,sendName)==0)
						{
							receiver = users[i];
							found = 1;
							break;
						}
					}
					if(found == 0)
					{
						sem_post(&sem_user);
						free(inputCopy);
						continue;	
					}
					char *length = strtok_r(NULL,"\n", &remainder);
					for(int i=0; i<strlen(length); ++i)
					{
						if(!isdigit(length[i]))
						{
							found = 0;
							break;
						}
					}
					if(found == 0)
					{
						free(inputCopy);
						sem_post(&sem_user);
						continue;
					}
					/*
							
							TAKE IN DATA FROM SENDER

					*/

					char *forNewClient = calloc(9+strlen(account->username)+strlen(length), sizeof(char));
					strcat(forNewClient, "SHARE ");
					strcat(forNewClient, account->username);
					strcat(forNewClient, " ");
					strcat(forNewClient, length);
					strcat(forNewClient, "\n");
					forNewClient[strlen(forNewClient)]= '\0';
					send(receiver->fd, forNewClient, strlen(forNewClient), 0);
					free(forNewClient);

					int size = atoi(length);
					char *fileData = calloc(size, sizeof(unsigned char));
					char shareBuffer[1024];
					send(*threadSock, "OK!\n", 4, 0);
					int numRecv = 0;
					int readNum = 0;
					//printf("%d\n", size);
					while(numRecv < size)
					{
						if(size - numRecv < 1024)
						{
							readNum = recv(*threadSock, shareBuffer, size-numRecv, 0);
							send(receiver->fd, shareBuffer, size-numRecv, 0);
							memcpy(fileData+numRecv, shareBuffer, size-numRecv);
						}
						else
						{
							readNum = recv(*threadSock, shareBuffer, 1024, 0);
							send(receiver->fd, shareBuffer, 1024, 0);
							memcpy(fileData+numRecv, shareBuffer, 1024);
						}
						send(*threadSock, "OK!\n", 4, 0);
						numRecv = numRecv + readNum;
					}

					//printf("HEYYY %d\n", numRecv);
					/*

						SEND DATA TO RECEIVER

					*/

					//printf("%s\n", fileData);
					numRecv = 0;
					//printf("%s\n", length);
					//int i=0;
					//printf("LENGTH %ld\n", strlen(fileData));
					//printf("%s\n", fileData);
					/*
					while(numRecv < size)
					{
						if(size - numRecv < 1024)
						{
							readNum = send(receiver->fd, fileData+numRecv, size-numRecv, 0);
							numRecv = numRecv + readNum;
						}
						else
						{
							readNum = send(receiver->fd, fileData+numRecv, 1024, 0);
							numRecv = numRecv + readNum;
						}
						//printf("%d %d\n", i, numRecv);
						++i;
					}
					*/
					sem_post(&sem_user);
					//printf("%d\n", numRecv);
					free(fileData);
				}
			}
			else
			{
				if((strcmp(command, "BROADCAST") == 0 || broadcastMessage == 1) && sendMessage == 0)
				{
					//BROADCAST
					if(broadcastMessage == 0)
					{
						printf("CHILD %ld: Rcvd BROADCAST request\n", pthread_self());
						broadcastMessage = 1;
						msglenStr = strtok_r(NULL, "\n", &remainder);
						sem_wait(&sem_user);
						for(int i=0; i<strlen(msglenStr); ++i)
						{
							if(!isdigit(msglenStr[i]))
							{
								send(*threadSock, "ERROR Invalid BROADCAST format\n", 31, 0); 
								broadcastMessage = 0;
								printf("CHILD %ld: Sent ERROR (Invalid BROADCAST format)\n", pthread_self());
								break;
							}
						}
						if(broadcastMessage == 0)
						{
							sem_post(&sem_user);
							continue;
						}
						msglen = malloc(sizeof(int));
						*msglen = atoi(msglenStr);
						if(*msglen > 990 || *msglen < 1)
						{
							send(*threadSock, "ERROR Invalid msglen\n", 21, 0);
							printf("CHILD %ld: Sent ERROR (Invalid msglen)\n", pthread_self());
							free(msglen);
							free(inputCopy);
							sem_post(&sem_user);
							continue;
						}
						char * newMessage = calloc(*msglen+1, sizeof(char));
						strcat(newMessage,remainder);
						while(strlen(newMessage) < *msglen)
						{
							char tempBuffer[1024];
							recv(*threadSock, tempBuffer,1024, 0);
							memcpy(newMessage+strlen(newMessage), tempBuffer, *msglen-strlen(newMessage));
						}
						newMessage[strlen(newMessage)] = '\0';
						send(account->fd, "OK!\n", 4, 0);
						broadcast(newMessage, msglenStr, account->username);
						broadcastMessage = 0;
						free(msglen);
						free(newMessage);
						sem_post(&sem_user);
					}
					else
					{
						//broadcast(buffer,*msglen);
						broadcastMessage = 0;
						free(msglen);
					}
				}
				else
				{
					if((strcmp(command, "SEND") == 0 || sendMessage == 1) && broadcastMessage == 0)
					{
						if(account == NULL)
						{
							send(*threadSock, "Not logged in\n", 14, 0);
						}
						else
						{
							//SEND
							if(sendMessage == 0)
							{
								msglenStr = strtok_r(NULL, " ", &remainder);
								printf("CHILD %ld: Rcvd SEND request to userid %s\n", pthread_self(), msglenStr);
								sem_wait(&sem_user);
								userProfile *receiver;
								for(int i=0; i<numUsers; ++i)
								{
									if(strcmp(users[i]->username, msglenStr) == 0)
									{
										receiver = users[i];
										sendMessage = 1;
										break;
									}
								}
								if(sendMessage == 0)
								{
									printf("CHILD %ld: Sent ERROR (Unknown userid)\n", pthread_self());
									send(*threadSock, "ERROR Unknown userid\n", 21, 0); 
									free(inputCopy);
									sem_post(&sem_user);
									continue;
								}
								person = malloc((strlen(msglenStr)+1)*sizeof(char));
								strcpy(person,msglenStr);
								person[strlen(msglenStr)] = '\0';
								msglenStr = strtok_r(NULL, "\n", &remainder);
								for(int i=0; i<strlen(msglenStr); ++i)
								{
										if(!isdigit(msglenStr[i]))
										{
											send(*threadSock, "ERROR Invalid SEND format\n", 26, 0);
											printf("CHILD %ld: Sent ERROR (Invalid SEND format)\n", pthread_self());
											free(person);
											free(inputCopy);
											sendMessage = 0;
											sem_post(&sem_user);
											break;
										}
								}
								if(sendMessage == 0)
								{
									continue;
								}
								msglen = malloc(sizeof(int));
								*msglen = atoi(msglenStr);
								if(*msglen > 990 || *msglen < 1)
								{
									send(*threadSock, "ERROR Invalid msglen\n", 21, 0);
									printf("CHILD %ld: Sent ERROR (Invalid msglen)\n", pthread_self());
									free(person);
									free(msglen);
									free(inputCopy);
									sendMessage = 0;
									sem_post(&sem_user);
									continue;
								}
								char * newMessage = calloc(*msglen+1, sizeof(char));
								strcat(newMessage,remainder);
								int num = strlen(newMessage);
			
								
								while(strlen(newMessage) < *msglen)
								{
									char tempBuffer[1024];
									int rc = recv(*threadSock, tempBuffer,1024, 0);
									//send(receiver->fd, tempBuffer, strlen(tempBuffer), 0);
									memcpy(newMessage+strlen(newMessage), tempBuffer, *msglen-strlen(newMessage));
									num = num + rc;
								}
								newMessage[strlen(newMessage)] = '\0';
								
								send(*threadSock, "OK!\n", 4, 0);
								char *forNewClient = calloc(9+strlen(account->username)+strlen(msglenStr)+strlen(newMessage), sizeof(char));
								strcat(forNewClient, "FROM ");
								strcat(forNewClient, account->username);
								strcat(forNewClient, " ");
								strcat(forNewClient, msglenStr);
								strcat(forNewClient, " ");
								strcat(forNewClient, newMessage);
								strcat(forNewClient, "\n");
								forNewClient[strlen(forNewClient)]= '\0';
								send(receiver->fd, forNewClient, strlen(forNewClient), 0);
								
								//sendMsg(newMessage,msglenStr,person,account->username);
								free(forNewClient);
								free(msglen);
								free(person);
								free(newMessage);
								sendMessage = 0;
								sem_post(&sem_user);
							}
							else
							{
								//sendMsg(buffer,*msglen, person);
								sendMessage = 0;
								send(*threadSock, "OK!\n", 4, 0);
								free(msglen);
								free(person);
							}
						}
					}
					else
					{
						//NOTHING
						printf("NOTHING\n");
					}
				}
			}
		}
		free(inputCopy);
	}
	printf("CHILD %ld: Client disconnected\n", pthread_self());
	if(account != NULL)
	{
		logout(account->username);
	}
	free(threadSock);
	pthread_detach(pthread_self());
	return NULL;
}

int main(int argc, char **argv)
{
	setvbuf( stdout, NULL, _IONBF, 0 );
	if(argc < 2)
	{
		return EXIT_FAILURE;
	}

	for(int i=0; i<strlen(argv[1]); ++i)
	{
		if(!isdigit(argv[1][i]))
		{
			return EXIT_FAILURE;
		}
	}
	signal(SIGINT, signal_handler);
	numUsers = 0;
	if(sem_init(&sem_user, 0, 1)==-1)
	{
		fprintf(stderr, "semaphore failed.\n");
		return EXIT_FAILURE;
	}

  /* ====== */
  fd_set readfds;
  //int client_sockets[ MAX_CLIENTS ]; /* client socket fd list */
  //int client_socket_index = 0;  /* next free spot */
  /* ====== */


  /* Create the listener socket as TCP socket */
  /*   (use SOCK_DGRAM for UDP)               */
  int sock = socket( PF_INET, SOCK_STREAM, 0 );
    /* note that PF_INET is protocol family, Internet */

  if ( sock < 0 )
  {
    perror( "socket()" );
    exit( EXIT_FAILURE );
  }

  /* socket structures from /usr/include/sys/socket.h */
  struct sockaddr_in server;
  struct sockaddr_in client;

  server.sin_family = PF_INET;
  server.sin_addr.s_addr = INADDR_ANY;

  unsigned short port = atoi(argv[1]);

  /* htons() is host-to-network-short for marshalling */
  /* Internet is "big endian"; Intel is "little endian" */
  server.sin_port = htons( port );
  int len = sizeof( server );

  if ( bind( sock, (struct sockaddr *)&server, len ) < 0 )
  {
    perror( "bind()" );
    exit( EXIT_FAILURE );
  }

  listen( sock, 5 );  /* 5 is number of waiting clients */
	
	/* create UDP socket */
  int udpfd = socket(AF_INET, SOCK_DGRAM, 0); 
  // binding server addr structure to udp sockfd 
  bind(udpfd, (struct sockaddr*)&server, len); 	
	FD_SET(udpfd, &readfds);
  int fromlen = sizeof( client );

  char buffer[ BUFFER_SIZE ];

  int n;
	
	printf("MAIN: Started server\n");
	printf("MAIN: Listening for TCP connections on port: %d\n", port);
	printf("MAIN: Listening for UDP datagrams on port: %d\n", port);

  while ( 1 )
  {
		pthread_t tid;
#if 1
    struct timeval timeout;
    timeout.tv_sec = 2;
    timeout.tv_usec = 500;  /* 2 seconds AND 500 microseconds */
#endif

    FD_ZERO( &readfds );
    FD_SET( sock, &readfds );   /* listener socket, fd 3 */
    FD_SET( udpfd, &readfds);
		//printf( "Set FD_SET to include listener fd %d\n", sock );

    /* initially, this for loop does nothing; but once we have */
    /*  client connections, we will add each client connection's fd */
    /*   to the readfds (the FD set) */
		/*
    for ( i = 0 ; i < client_socket_index ; i++ )
    {
      FD_SET( client_sockets[ i ], &readfds );
      printf( "Set FD_SET to include client socket fd %d\n",
              client_sockets[ i ] );
    }
		*/

#if 0
    /* This is a BLOCKING call, but will block on all readfds */
    int ready = select( FD_SETSIZE, &readfds, NULL, NULL, NULL );
#endif

#if 1
    select( FD_SETSIZE, &readfds, NULL, NULL, &timeout );
#endif

    /* ready is the number of ready file descriptors */
    //printf( "select() identified %d descriptor(s) with activity\n", ready );


    /* is there activity on the listener descriptor? */
    if ( FD_ISSET( sock, &readfds ) )
    {
      int newsock = accept(sock, (struct sockaddr *)&client, (socklen_t *)&fromlen );
      /* this accept() call we know will not block */
      printf("MAIN: Rcvd incoming TCP connection from: %s\n", inet_ntoa(client.sin_addr));
			int* newsockPointer = malloc(sizeof(int));
			*newsockPointer = newsock;
      //client_sockets[ client_socket_index++ ] = newsock;
			int rc = pthread_create(&tid, NULL, thread_function, newsockPointer);
			if(rc != 0)
			{
				fprintf(stderr, "FAILED TO CREATE THREAD\n");
				return EXIT_FAILURE;
			}
    }

		if (FD_ISSET(udpfd, &readfds))
		{
			socklen_t clientlen = sizeof(client); 
      bzero(buffer, sizeof(buffer)); 
      n = recvfrom(udpfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&client, &clientlen);  
      //sendto(udpfd, "HI", 2*sizeof(char), 0, (struct sockaddr*)&client, sizeof(client));
      printf("MAIN: Rcvd incoming UDP datagram from: %s\n", inet_ntoa(client.sin_addr)); 
			if(buffer[n-1] == '\n')
			{
				buffer[n-1] = '\0';
			}
			else
			{
				buffer[n] = '\0';
			}
			
			if(strcmp(buffer,"WHO") == 0)
			{
				printf("MAIN: Rcvd WHO request\n");
				char* list = who();
				sendto(udpfd, list, strlen(list)*sizeof(char), 0, (struct sockaddr*) &client, sizeof(client));
				free(list);
				continue;
			}

			char *command;
			char *remainder;
			char *inputCopy = (char*) calloc(n + 1, sizeof(char));
			strncpy(inputCopy, buffer, n);
			command = strtok_r(inputCopy, " ", &remainder);
			if(strcmp(command,"BROADCAST") == 0)
			{
				char *msglenStr = strtok_r(NULL, "\n", &remainder);
				int canSend = 1;
				printf("MAIN: Rcvd BROADCAST request\n");
				sem_wait(&sem_user);
				for(int i=0; i<strlen(msglenStr); ++i)
				{
					if(!isdigit(msglenStr[i]))
					{
						sendto(udpfd, "ERROR Invalid BROADCAST format\n", 31*sizeof(char), 0, (struct sockaddr*) &client, sizeof(client)); 
						printf("MAIN: Sent ERROR (Invalid BROADCAST format)\n");
						canSend = 0;
					}
				}
					if(canSend == 0)
					{
						sem_post(&sem_user);
						free(inputCopy);
						continue;
					}
					int test = atoi(msglenStr);
					if(test > 990 || test < 1)
					{
						sendto(udpfd, "ERROR Invalid msglen\n", 21*sizeof(char), 0, (struct sockaddr*) &client, sizeof(client));
						printf("MAIN: Sent ERROR (Invalid msglen)\n");
						free(inputCopy);
						sem_post(&sem_user);
						continue;
					}
					char * newMessage = calloc(test+1, sizeof(char));
					strcat(newMessage,remainder);
					while(strlen(newMessage) < test)
					{
						char tempBuffer[1024];
      			recvfrom(udpfd, tempBuffer, sizeof(tempBuffer), 0, (struct sockaddr*)&client, &clientlen);  
						memcpy(newMessage+strlen(newMessage), tempBuffer, test-strlen(newMessage));
					}
					newMessage[strlen(newMessage)] = '\0';
					sendto(udpfd, "OK!\n", 4, 0, (struct sockaddr*) &client, sizeof(client));
					broadcast(newMessage, msglenStr, "UDP-client");
					free(newMessage);
					sem_post(&sem_user);
			}
			else
			{
				if(strcmp(command,"SEND") == 0)
				{
					sendto(udpfd, "ERROR SEND not supported over UDP\n", 34*sizeof(char), 0, (struct sockaddr*) &client, sizeof(client)); 
					printf("MAIN: Sent ERROR (SEND not supported over UDP)\n");
				}
			}
			free(inputCopy);
		}
		
  }

  return EXIT_SUCCESS; /* we never get here */
}
