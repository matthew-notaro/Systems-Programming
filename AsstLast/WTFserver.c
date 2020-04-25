#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
//#include <arpa/inet.h>

int connectToClient();

int main(int argc, char **argv)
{
	return 0;
}

int connectToClient()
{
	char buffer[256];
	int sockfd, bindSocket, listen, newsockfd;
	struct sockaddr_in serverAddressInfo;
	struct sockaddr_in clientAddressInfo;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd < 0)
		printf("ERROR: Socket does not exist.\n");

	bzero(&serverAddressInfo, sizeof(serverAddressInfo));

	serverAddressInfo.sin_family = AF_INET;
	serverAddressInfo.sin_addr.s_addr = htons(INADDR_ANY);
	serverAddressInfo.sin_port = htons(PORT);

	bindSocket = bind(sockfd, (struct sockaddr *)&serverAddressInfo, sizeof(serverAddressInfo));

	if (bindSocket < 0)
		printf("ERROR: Could not bind.\n");

	listen = listen(sockfd, 0);

	if (listen < 0)
		printf("ERROR: Could not listen.\n");

	// keep running until listening stops
	while (1)
	{
		newsockfd = accept(sockfd, (struct sockaddr *)&clientAddressInfo, sizeof(clientAddressInfo));

		if (cxnfd < 0)
			printf("ERROR: Socket not accepted.\n");

		//pthread_create
		//pthread_join
	}
}