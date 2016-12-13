#ifndef REQUEST_H
#define REQUEST_H

#include <unistd.h> /* read, write, close */
#include <string.h> /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h> /* struct hostent, gethostbyname */

#include "Color.h"

#define DATA_SIZE 1000//1048576 //3145728 // 1 mb = 1024*1024 bytes
#define FRAGMENT_SIZE 200//104857 //
#define TIME_TO_SLEEP 1 // in seconds

typedef struct hostent HostEntity;
typedef struct sockaddr_in SocketAddress;

void threadError(char* thread, const char *msg)
{
  char* aux = malloc( sizeof(thread) + sizeof(msg) + 100 );
  strcpy(aux, thread);
  strcat(aux, msg);
  //printf("%s", msg);
	perror(aux);
	//exit(0);
}

void error(const char *msg)
{
  //printf("%s", msg);
	perror(msg);
	//exit(0);
}



#endif
