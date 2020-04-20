#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>


int main(int argc, char** argv){

	char buffer[256];
  int sockfd, cxnfd, bindSocket, listen;
  struct sockaddr_in serverAddressInfo;
    
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
	if(sockfd < 0)
		printf("ERROR: Socket does not exist.\n");
 
  bzero(&serverAddressInfo, sizeof(serverAddressInfo));
 
  serverAddressInfo.sin_family = AF_INET;
  serverAddressInfo.sin_addr.s_addr = htons(INADDR_ANY);
  serverAddressInfo.sin_port = htons(PORT);
    
  bindSocket = bind(sockfd, (struct sockaddr *) &serverAddressInfo, sizeof(serverAddressInfo));
	
	if(bindSocket < 0)
		printf("ERROR: Could not bind.\n");
 
  listen = listen(sockfd, 0);
	
	if(listen < 0)
		printf("ERROR: Could not listen.\n");
    
  cxnfd = accept(sockfd, (struct sockaddr*) &serverAddressInfo, sizeof(serverAddressInfo));
	
	if(cxnfd < 0)
		printf("ERROR: No socket.\n");
 
	// keep running until listening stops
	while(1)
	{

	}
}
