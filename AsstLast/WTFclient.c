#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>

//#include <arpa/inet.h>

int connectToServer();
int configure(char* IPAddress, char* portNum);

int main(int argc, char **argv) 
{
	char* op = argv[0];
	
	return 0;
}

int configure(char* IPAddress, char* portNum)
{
	char* configName = ".configure";
	int IPAddressSize = 0, portNumSize = 0;
	
	int fd = open(configName, O_RDWR|O_CREAT|O_APPEND, 00600);
	
  if(fd < 0)
	{
    printf("File does not exist.\n"); 
    return -1;
  }
	IPAddressSize = strlen(IPAddress) + 1;
	portNumSize = strlen(portNum) + 1;
	
	write(fd, IPAddress, IPAddressSize);
	write(fd, "	", sizeof(char)*1);
	write(fd, portNum, portNumSize);

	return 0;
}

int connectToServer()
{
	//all from my notes lmao 
	
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
	struct hostent* result = gethostbyname();
	
	struct sockaddr_in serverAddress;
	
	bzero(&serverAddress, sizeof(serverAddress));
	
	serverAddressInfo.sin_family = AF_INET;
	serverAddressInfo.sin_addr.s_addr = htons(INADDR_ANY);
	serverAddressInfo.sin_port = htons(7621);
	
	bcopy((char*)result->h_addr, (char*)&serverAddress.sin_addr.s_addr, result->addr_length);
	
	connect(sockfd, &serverAddress, sizeof(serverAddress));
	
	return 0;
}
