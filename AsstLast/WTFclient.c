#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>

//#define PORT 42069

char* HOST = NULL;
char* PORT = NULL;

int configure(char* IPAddress, char* portNum);
int checkout(char* project);
int update(char* project);
int upgrade(char* project);
int commit(char* project);
int push(char* project);
int create(char* project);
int destroy(char* project);
int add(char* project);
int remove_(char* project);
int update_(char* project);
int currentversion(char* project);
int history(char* project);
int rollback(char* project, char* version);

int setServerDetails();

int connectToServer();
int sendMessage();
char* readFromFile(char* file);


int main(int argc, char **argv) 
{
	char* op = argv[0];
	//configure("sampleIP", "samplePort");
	setServerDetails();
	printf("host: %s\n", HOST);
	printf("port: %s\n", PORT);
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

int create(char* project)
{
	return 0;
}

int destroy(char* project)
{
	return 0;
}

int add(char* project)
{
	return 0;
}

int remove_(char* project)
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

int setServerDetails()
{
	char* configure_string = readFromFile("./.configure");
	int len = strlen(configure_string), spaceFound = 0, i = 0, j = 0;
	
	for(i = 0; i < len; i++)
	{
		if(isspace(configure_string[i]) != 0)
		{
			spaceFound = 1;
			break;
		}
	}

	if(spaceFound == 1)
	{
		int hostLen = i;
		int portLen = len-i-1;
		char* host_buffer = malloc(hostLen);
		char*	port_buffer = malloc(portLen);
		
		//Extracts host name / IP address
		for(j = 0; j < hostLen; j++)
		{
			host_buffer[j] = configure_string[j];
		}
		
		//Extracts port
		for(j = 0; j < portLen; j++)
		{
			port_buffer[j] = configure_string[j+1+i];
		}
		
		HOST = host_buffer;
		PORT = port_buffer;
		return 0;
	}
	
	else
	{
		return -1;
	}
}

int connectToServer()
{
	int sockfd = 0, cxn_status = 0; 
	char* buffer;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
	if(sockfd < 0)
	{
		printf("ERROR\n");
		return -1;
	}
		
	struct hostent* hostIP = gethostbyname((char*)PORT);
	struct sockaddr_in serverAddressInfo;
	
	bzero((char*)&serverAddressInfo, sizeof(serverAddressInfo));
	
	serverAddressInfo.sin_family = AF_INET;
	serverAddressInfo.sin_addr.s_addr = htons(INADDR_ANY);
	serverAddressInfo.sin_port = htons(12342);
	
	bcopy((char*)host->h_addr, (char*)&serverAddressInfo.sin_addr.s_addr, host->h_length);
	
	
	cxn_status = connect(sockfd, (struct sockaddr *)&serverAddressInfo, sizeof(serverAddressInfo));
	if(cxn_status < 0)
	{
		printf("ERROR\n");
		return -1;
	}
	
	printf("%d\n", cxn_status);
	printf("checkpoint\n");
	
	//buffer = malloc(9);
	buffer = "test.txt";
	sendMessage(sockfd, buffer);
	
	return 0;
}

int sendMessage(int sockfd, char* buffer)
{
	int n = 0;
	//bzero(buffer,strlen(buffer));

	n = write(sockfd,buffer,strlen(buffer));
	if (n < 0) 
		 printf("ERROR writing to socket\n");
	//bzero(buffer,strlen(buffer));
	
	/*
	n = read(sockfd,buffer,strlen(buffer)-1);
	 if (n < 0) 
			printf("ERROR reading from socket\n");
	printf("%s\n",buffer);*/
	
	return 0;
}

// Read entire file into string buffer
// Returns NULL if file does not exist, string otherwise
char* readFromFile(char* file)
{
    int fd = open(file, O_RDONLY);    // Returns -1 on failure, >0 on success
    // Fatal Error if file does not exist
    if(fd < 0){
        printf("Fatal Error: File does not exist.\n");
        return NULL;
    }
    struct stat *buffer = malloc(sizeof(struct stat));
    if(buffer == NULL){
        printf("Bad malloc\n");
        return NULL;
    }
    stat(file, buffer);
    int buffer_size = buffer->st_size;
    // Warning: Empty file
    if(buffer_size == 0){
        printf("Warning: Empty file.\n");
    }
		
    // IO Read Loop
    char* file_buffer = (char*)malloc(buffer_size);
    if(file_buffer == NULL){
        printf("Bad malloc\n");
        return NULL;
    }
    memset(file_buffer, '\0', buffer_size);
    int status = 1;
    int readIn = 0;
    do{
        status = read(fd, file_buffer+readIn, buffer_size - readIn);
        readIn += status;
    } while(status > 0 && readIn < buffer_size);

    free(buffer);
    return file_buffer;
}

