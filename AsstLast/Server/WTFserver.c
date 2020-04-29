#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netdb.h>
#include <unistd.h>
#include <strings.h>
#include <netinet/in.h>
#include <pthread.h>
#include <fcntl.h>

int port = 0;

int checkout(char* project);
int update(char* project);
int upgrade(char* project);
int commit(char* project);
int push(char* project);
int create(char* project);
int destroy(char* project);
int update_(char* project);
int currentversion(char* project);
int history(char* project);
int rollback(char* project, char* version);

void* socketThread(void* sockfd);

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
char client_message[2000];

int main(int argc, char **argv){
	if(argc != 2){
		printf("ERROR: Please enter a valid port number\n");
		return -1;
	}
	port = atoi(argv[1]);
	if(port < 0 || port > 65535){
		printf("ERROR: Please enter a valid port number\n");
		return -1;
	}
	
	int sockfd, newsockfd, portno, clientLen;
	char buffer[256];
	struct sockaddr_in serverAddressInfo, clientAddressInfo;
	int n;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		printf("ERROR opening socket\n");
	bzero((char *)&serverAddressInfo, sizeof(serverAddressInfo));
	portno = port;
	serverAddressInfo.sin_family = AF_INET;
	serverAddressInfo.sin_addr.s_addr = INADDR_ANY;
	serverAddressInfo.sin_port = htons(portno);

	if (bind(sockfd, (struct sockaddr *)&serverAddressInfo, sizeof(serverAddressInfo)) < 0)
		printf("ERROR binding\n");
	listen(sockfd, 50);


	// THREADING STARTS NOW
	pthread_t tid[60];
	int i = 0;
	while(1){
		clientLen = sizeof(clientAddressInfo);
		newsockfd = accept(sockfd, (struct sockaddr *)&clientAddressInfo, (socklen_t *)&clientLen);
		if (newsockfd < 0)
		{
			printf("ERROR: Could not accept\n");
		}
		if(pthread_create(&tid[i++], NULL, socketThread, &newsockfd) != 0){
			printf("ERROR: Could not create thread\n");
		}

		// Deals with only having 60 possible threads at a time - can change later using linked lists
		if(i >= 50){
			i = 0;
			while(i < 50){
				pthread_join(tid[i++], NULL);
			}
			i = 0;
		}
	}
	return 0;
}

// Function to be made into a thread for each connection to server
void* socketThread(void* sockvoidstar){
	printf("Entering thread\n");
	int sock = *((int*)sockvoidstar);
	int n;
	char buffer[1024];
	bzero(client_message, 256);
	n = read(sock, client_message, 255);
	if (n < 0)
	{
		printf("ERROR: Could not read from socket\n");
	}

	// LOCK
	pthread_mutex_lock(&lock);
	char* message = (char*)malloc(sizeof(client_message));
	strcpy(message, "Message received: \n");
	strcat(message, client_message);
	strcat(message, "\n");
	strcpy(buffer, message);
	free(message);
	printf("Message: %s\n", buffer);
	pthread_mutex_unlock(&lock);
	// UNLOCK

	n = write(sock, buffer, 255);
	if (n < 0)
	{
		printf("ERROR: Could not write to socket\n");
	}
	printf("Exiting thread\n");
	close(sock);
	pthread_exit(NULL);
}


int checkout(char* project)
{
	return 0;
}

int update(char* project)
{
	return 0;
}

int upgrade(char* project)
{
	return 0;
}

int commit(char* project)
{
	return 0;
}

int push(char* project)
{
	return 0;
}

/*
The create command will fail if the project name already exists on the server or the client can not communicate
with the server. Otherwise, the server will create a project folder with the given name, initialize a .Manifest for it
and send it to the client. The client will set up a local version of the project folder in its current directory and
should place the .Manifest the server sent in it.
*/
int create(char* project)
{
	int status;
  status = mkdir(project, 00600); //CHECK WITHIN PROJECTS FOLDER
	if(status < 0)
	{
		char* manifestName = ".Manifest";
		int fd = open(manifestName, O_RDWR|O_CREAT|O_APPEND, 00600);
		//SEND MANIFEST TO CLIENT
	}
	else
	{
		printf("Error: Project already exists.");
	}
	return 0;
}

int destroy(char* project)
{
	return 0;
}

int update_(char* project)
{
	return 0;
}

int currentversion(char* project)
{
	return 0;
}

int history(char* project)
{
	return 0;
}

int rollback(char* project, char* version)
{
	return 0;
}