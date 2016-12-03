#ifndef MY_THREAD
#define MY_THREAD

/* Includes */
#include <pthread.h>

#include "Color.h"
#include "Request.h"

/* Structures */
typedef struct _threadBody
{
  int code; // know which thread i am
  char* name;

  int argc;
  char** argv;

  char* color;
} ThreadBody;

void DoIT(ThreadBody* tb);
void * ThreadBehaviour(void * tB);

void * ThreadBehaviour(void * tB)
{
  ThreadBody* threadBody = (ThreadBody*)tB;
  threadBody->color = GetColor(threadBody->code);

  //printf("%sI am thread %d%s\n", color, threadBody->code, KWHT);

  DoIT(threadBody);

  //printf("Value of %s is %d\n", argument, value);

	/*int i;
	for(i = 0; i < 10; i++)
	{
    printf("%sThread %d is sleeping for %d seconds.%s\n", color, threadBody->code, i, KNRM);
		sleep(1); // 1 second
	}*/

	return NULL;
}

void DoIT(ThreadBody * tb)
{
  int argc = tb->argc;
  char** argv = tb->argv;
  char* color = malloc(30);
  strcpy(color, tb->color);
  strcat(color, "Thread ");
  strcat(color, tb->name);
  strcat(color, " ");

  /* Local variables */
  int port;
  char* host;

  HostEntity *server;
  SocketAddress serv_addr;

  int sockfd; // socket file descriptor to server
  int bytes; // number of bytes read from / wrote to server
  int sent;
  int received;
  int total; // size of request and response message
  int requestSize;
  char* message;
  char* response[4096];
  char* myMessage;

  int fragmentNumber;

  /* Beggin of method */
	myMessage = malloc(DATA_SIZE);

	int i;
	myMessage[0] = 'v';
	myMessage[1] = 'a';
	myMessage[2] = 'r';
	myMessage[3] = '=';

	for(i = 4; i < DATA_SIZE; i++)
	{
		myMessage[i] = 'b';
	}

	/* first where are we going to send it? */
	host = strlen(argv[1]) > 0 ? argv[1] : "localhost";
  port = atoi(argv[2]) > 0 ? atoi(argv[2]) : 80;

  /* How big is the message? */
	requestSize = 0;

	if(!strcmp(argv[3],"GET"))
  {
		requestSize+=strlen("%s %s%s%s HTTP/1.0\r\n");        /* method         */
    requestSize+=strlen(argv[3]);                         /* path           */
    requestSize+=strlen(argv[4]);                         /* headers        */
    if(argc>5)
      requestSize+=strlen(myMessage);//argv[5]);                  /* query string   */
    for(i=6;i<argc;i++)                                    /* headers        */
      requestSize+=strlen(argv[i])+strlen("\r\n");
    requestSize+=strlen("\r\n");                          /* blank line     */
  }
  else
  {
    requestSize+=strlen("%s %s HTTP/1.0\r\n");
    requestSize+=strlen(argv[3]);                         /* method         */
    requestSize+=strlen(argv[4]);                         /* path           */
    for(i=6;i<argc;i++)                                    /* headers        */
      requestSize+=strlen(argv[i])+strlen("\r\n");
    if(argc>5)
      requestSize+=strlen("Content-Length: %d\r\n")+10; /* content length */
    requestSize+=strlen("\r\n");                          /* blank line     */
    if(argc>5)
      requestSize+=strlen(myMessage);//argv[5]);                     /* body           */
  }

  /* allocate space for the message */
  message = malloc(requestSize);

  /* fill in the parameters */
	if(!strcmp(argv[3],"GET"))
  {
    if(argc>5)
      sprintf(message,"%s %s%s%s HTTP/1.0\r\n",
        strlen(argv[3]) > 0 ? argv[3] : "GET",               /* method         */
        strlen(argv[4]) > 0 ? argv[4] : "/",                 /* path           */
        strlen(argv[5]) > 0 ? "?" : "",                      /* ?              */
        //strlen(argv[5])>0?argv[5]:"");                 /* query string   */
				strlen(myMessage) > 0 ? myMessage : "");
		else
      sprintf(message,"%s %s HTTP/1.0\r\n",
        strlen(argv[3])>0?argv[3]:"GET",               /* method         */
        strlen(argv[4])>0?argv[4]:"/");                /* path           */
    for(i=6;i<argc;i++)                                    /* headers        */
    {
			strcat(message,argv[i]);
			strcat(message,"\r\n");
		}
    strcat(message,"\r\n");                                /* blank line     */
  }
  else
  {
    sprintf(message,"%s %s HTTP/1.0\r\n",
      strlen(argv[3])>0?argv[3]:"POST",                  /* method         */
      strlen(argv[4])>0?argv[4]:"/");                    /* path           */

		for(i = 6; i<argc; i++)                                    /* headers        */
    {
			strcat(message,argv[i]);
			strcat(message,"\r\n");
		}

		if(argc>5)
      sprintf(message+strlen(message),"Content-Length: %d\r\n", strlen(myMessage));//argv[5]));

		strcat(message,"\r\n");                                /* blank line     */
    if(argc>5)
      strcat(message, myMessage);//argv[5]);                           /* body           */
  }

	/*
	int requestBuildedSize = BuildRequest(message, argc, argv);
	if(requestBuildedSize != requestSize) // requestSize > requestBuildedSize
	{
		printf("RequestSize: %d\n", requestSize);
		printf("RequestBuildedSize: %d\n", requestBuildedSize);

		printf("wtf\n");
		exit(0);
	}
	*/

  /* What are we going to send? */
  //printf("Request:\n%s\n", message);

  /* create the socket */
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
		threadError(tb->name, " ERROR opening socket");

  printf("%sSocket opened: %d%s\n", color, sockfd, KNRM);

  /* lookup the ip address */
  server = gethostbyname(host);
  if (server == NULL)
		threadError(tb->name, " ERROR, no such host");

  /* fill in the structure */
  memset(&serv_addr,0,sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port);
  memcpy(&serv_addr.sin_addr.s_addr,server->h_addr,server->h_length);

  /* connect the socket */
  if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
    threadError(tb->name, " ERROR connecting");

  /* send the request */
  total = strlen(message);
  sent = 0;

  fragmentNumber = 0;
  do
	{
    bytes = write(sockfd, message+sent, FRAGMENT_SIZE);//total-sent);
    if (bytes < 0)
    {
      threadError(tb->name, " ERROR writing message to socket");
      break;
    }
    if (bytes == 0)
      break;
    sent += bytes;

    fragmentNumber++;

    printf("%sFragment %d sent.%s\n", color, fragmentNumber, KNRM);

		sleep(TIME_TO_SLEEP);
  } while (sent < total);

	/****************************************/
	/****************************************/
	/****************************************/

  /* receive the response */
  memset(response, 0, sizeof(response));
  total = sizeof(response)-1;
  received = 0;
  do
	{
    bytes = read(sockfd,response+received,total-received);
    if (bytes < 0)
      threadError(tb->name, " ERROR reading response from socket");
    if (bytes == 0)
      break;
    received+=bytes;
  } while (received < total);

  if (received == total)
    threadError(tb->name, " ERROR storing complete response from socket");

  /* close the socket */
  close(sockfd);

  /* process response */
  printf("%sResponse:\n%s%s\n", color, response, KNRM);

  free(message);
}

#endif
