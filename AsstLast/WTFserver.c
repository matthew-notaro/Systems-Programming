#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <strings.h>

//#include <arpa/inet.h>

#define PORT 42069

int connectToClient();

int main(int argc, char **argv){
	connectToClient();
	return 0;
}

int connectToClient(){
	char buffer[256];
	int sockfd, bindSocket, listen, newsockfd, clientLen;
	struct sockaddr_in serverAddressInfo;
	struct sockaddr_in clientAddressInfo;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0){
		printf("ERROR: Socket does not exist.\n");
	}

	bzero(((char*)&serverAddressInfo, sizeof(serverAddressInfo));
	serverAddressInfo.sin_family = AF_INET;
	serverAddressInfo.sin_addr.s_addr = htons(INADDR_ANY);
	serverAddressInfo.sin_port = htons(PORT);

	bindSocket = bind(sockfd, (struct sockaddr *)&serverAddressInfo, sizeof(serverAddressInfo));
	if (bindSocket < 0){
		printf("ERROR: Could not bind.\n");
	}
	listen = listen(sockfd, 5);
	if (listen < 0){
		printf("ERROR: Could not listen.\n");
	}
	clientLen = sizeof(clientAddressInfo);
	newsockfd = accpet(sockfd, (struct sockaddr*) &clientAddressInfo, &clientLen);
	if(newsockfd < 0){
		printf("ERROR: Could not accept\n");
	}
	bzero(buffer, 256);
	int n = read(newsockfd, buffer, 255);
	if(n < 0){
		printf("ERROR: Could not read from socket\n");
	}
	printf("Message: %s\n", buffer);
	n = write(newsockfd, "Message received", 16);
	if(n < 0){
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