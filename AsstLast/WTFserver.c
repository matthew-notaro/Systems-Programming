#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <strings.h>
#include <netinet/in.h>

//#include <arpa/inet.h>

#define PORT 42069

int connectToClient();

int main(int argc, char **argv)
{
	return connectToClient();
}

int connectToClient(){
	int sockfd, newsockfd, portno, clientLen;
	char buffer[256];
	struct sockaddr_in serverAddressInfo, clientAddressInfo;
	int n;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		printf("ERROR opening socket");
	bzero((char *)&serverAddressInfo, sizeof(serverAddressInfo));
	portno = PORT;
	serverAddressInfo.sin_family = AF_INET;
	serverAddressInfo.sin_addr.s_addr = INADDR_ANY;
	serverAddressInfo.sin_port = htons(portno);

	if (bind(sockfd, (struct sockaddr *)&serverAddressInfo, sizeof(serverAddressInfo)) < 0)
		printf("error");
	listen(sockfd, 5);

	clientLen = sizeof(clientAddressInfo);
	newsockfd = accept(sockfd, (struct sockaddr *)&clientAddressInfo, (socklen_t *)&clientLen);
	if (newsockfd < 0)
	{
		printf("ERROR: Could not accept\n");
	}

	bzero(buffer, 256);
	n = read(newsockfd, buffer, 255);
	if (n < 0)
	{
		printf("ERROR: Could not read from socket\n");
	}
	printf("Message: %s\n", buffer);
	n = write(newsockfd, "Message received", 16);
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