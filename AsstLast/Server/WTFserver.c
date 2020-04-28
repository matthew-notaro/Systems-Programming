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

int connectToClient();
int readFromClient(int sockfd);

int main(int argc, char **argv)
{
	if(argc != 2){
		printf("ERROR: Please enter a valid port number\n");
		return -1;
	}
	port = atoi(argv[1]);
	if(port < 0 || port > 65535){
		printf("ERROR: Please enter a valid port number\n");
		return -1;
	} 
	int sockfd = connectToClient();
	readFromClient(sockfd);



	return 0;
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

// Returns newsockfd
int connectToClient(){
	int sockfd, newsockfd, portno, clientLen;
	struct sockaddr_in serverAddressInfo, clientAddressInfo;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		printf("ERROR opening socket");
	bzero((char *)&serverAddressInfo, sizeof(serverAddressInfo));
	portno = port;
	serverAddressInfo.sin_family = AF_INET;
	serverAddressInfo.sin_addr.s_addr = INADDR_ANY;
	serverAddressInfo.sin_port = htons(portno); //CHANGE BACK TO PORTNO

	if (bind(sockfd, (struct sockaddr *)&serverAddressInfo, sizeof(serverAddressInfo)) < 0)
		printf("error");
	listen(sockfd, 5);

	clientLen = sizeof(clientAddressInfo);
	newsockfd = accept(sockfd, (struct sockaddr *)&clientAddressInfo, (socklen_t *)&clientLen);
	if (newsockfd < 0)
	{
		printf("ERROR: Could not accept\n");
	}
	return newsockfd;
}

int readFromClient(int sockfd){
	
	char buffer[256];
	int n;
	bzero(buffer, 256);
	n = read(sockfd, buffer, 255);
	if (n < 0)
	{
		printf("ERROR: Could not read from socket\n");
	}
	printf("Message: %s\n", buffer);
	n = write(sockfd, "Message received", 16);
	if (n < 0)
	{
		printf("ERROR: Could not write to socket\n");
	}

	// to accept multple connections - keep running until listening stops
	/*
	while (1)
	{
		newsockfd = accept(sockfd, (struct sockaddr *)&clientAddressInfo, sizeof(clientAddressInfo));

		if (cxnfd < 0)
			printf("ERROR: Socket not accepted.\n");

		//pthread_create
		//pthread_join
	}*/
	
	return 0;
}