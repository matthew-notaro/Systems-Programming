#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>

#define PORT 42069

int connectToServer();
int configure(char* IPAddress, char* portNum);

int main(int argc, char **argv) 
{
	char* op = argv[0];
	connectToServer();
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
	int sockfd = 0, cxn_status = 0, n = 0; 
	char buffer[256];

	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
	if(sockfd < 0)
	{
		printf("ERROR\n");
		return -1;
	}
	struct hostent* host = gethostbyname("127.0.0.1");
	struct sockaddr_in serverAddressInfo;

	bzero((char*)&serverAddressInfo, sizeof(serverAddressInfo));
	bzero(buffer,256);
	
	serverAddressInfo.sin_family = AF_INET;
	serverAddressInfo.sin_addr.s_addr = htons(INADDR_ANY);
	serverAddressInfo.sin_port = htons(PORT);
	
	bcopy((char*)host->h_addr, (char*)&serverAddressInfo.sin_addr.s_addr, host->h_length);
	
	
	cxn_status = connect(sockfd, (struct sockaddr *)&serverAddressInfo, sizeof(serverAddressInfo));
	if(cxn_status < 0)
	{
		printf("ERROR\n");
		return -1;
	}
	
	printf("%d\n", cxn_status);
	printf("checkpoint\n");
	
	printf("Please enter the message: \n");
	fgets(buffer,255,stdin);
	
  n = write(sockfd,buffer,strlen(buffer));
  if (n < 0) 
     printf("ERROR writing to socket\n");
  bzero(buffer,256);
  n = read(sockfd,buffer,255);
   if (n < 0) 
      printf("ERROR reading from socket\n");
  printf("%s\n",buffer);
	return 0;
}