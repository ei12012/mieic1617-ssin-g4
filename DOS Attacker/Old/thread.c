/*
now:
./thread 172.30.31.181 1234 POST /test "var=bife" "Content-Type: application/x-www-form-urlencoded"

goal:
./thread conf.txt
	IP ADDRESS
	PORT
	HTTP METHOD
	PATH
	HC "var=bife"
	TIME_TO_SLEEP
	DATA_MAX_SIZE
	FRAGMENT_MAX_SIZE
	HC "Content-Type: application/x-www-form-urlencoded"
*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "ThreadBody.h"
#include <pthread.h>

#define numberThreads 150
pthread_t myThreads[numberThreads];

/*
for index in argv
	0 : program name
	1 : ip address of server
	2 : port
	3 : http method (GET / POST)
	4 : path to content requested
		ex: /
				/users/
				/admin/users.php
	5 : parameters of http method or data
		ex: var=bife
	>5: http headers
	6 : http header content-type
	...
*/
int main(int argc, char** argv)
{
	if (argc < 5)
	{
		puts("Parameters: <host> <port> <method> <path> [<data> [<headers>]]");
		exit(0);
	}

	system("clear");

	int i;
	for(i = 0; i < numberThreads; i++)
	{
		ThreadBody* threadBody = (ThreadBody*)malloc(sizeof(ThreadBody));
		threadBody->code = i;
		threadBody->name = malloc(3);
		sprintf(threadBody->name, "%d", (i+1));

		threadBody->argc = argc;
		threadBody->argv = argv;

		printf("Thread %d created\n", i );
		if(pthread_create(&myThreads[i], NULL, ThreadBehaviour, threadBody))
		{
			fprintf(stderr, "Error creating thread\n");
			return -1;
		}

	}

	// main waits for other threads to end
	for(i = 0; i < numberThreads; i++)
	{
		pthread_join(myThreads[i], NULL);
	}

	printf("Main over\n");

	return(0);
}
