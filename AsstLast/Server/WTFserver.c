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
void recursion(char* name);

typedef struct file{
	char* fileName;
	int nameLen;
	int fileLen;
	char* fileData;
	struct file* next;
} file;

char* readNBytes(int fd, int numBytes);
char* readUntilDelim(int fd, char delim);
char* readFromFile(char* file);
char* intToString(int num);
void writeLoop(int fd, char* str, int numBytes);
void transferOver(int sockfd, file* fileLL, char* command);
file* addFileToLL(file* fileLL, char* name);

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


void recursion(char* name){
  DIR* currentDir = opendir(name);
    if(currentDir == NULL){
    printf("Error: invalid path\n");
    return;
  }
  struct dirent* currentThing = NULL;
  readdir(currentDir);
  readdir(currentDir);
  while((currentThing = readdir(currentDir)) != NULL){
    char buff[1024];
    snprintf(buff, sizeof(buff), "%s/%s", name, currentThing->d_name);
    if(currentThing->d_type == DT_REG){
      doOp(buff);
    }
    else if(currentThing->d_type == DT_DIR){
      recursion(buff);
    }
  }
  closedir(currentDir);
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


void yeetProject(char* project){

}


// Reads given n bytes from given fd
// Returns string of n bytes stored on the heap
// Returns NULL if 0 bytes requested
char* readNBytes(int fd, int numBytes){
    if(numBytes == 0){
		return NULL;
	}
	//Mallocs and memsets file buffer for actual file contents
    char* nBytes = (char*)malloc(numBytes*sizeof(char));
    if(nBytes == NULL){
        printf("Bad malloc\n");
        return NULL;
    }
    memset(nBytes, '\0', numBytes);

    // IO Read Loop
    int status = 1;
    int readIn = 0;
    do{
        status = read(fd, nBytes+readIn, numBytes - readIn);
        readIn += status;
    } while(status > 0 && readIn < numBytes);
    return nBytes;
}


// Reads byte by byte from fd until the delim is found
// Returns string of at most bufLen bytes stored on the heap
char* readUntilDelim(int fd, char delim){
	// Allocate 100 bytes - should be enough to store any file name or line of .Manifest
	int bufLen = 100;
	char* buffer = (char*)malloc(bufLen*sizeof(char));
	if(buffer == NULL){
        printf("Bad malloc\n");
        return NULL;
    }
	memset(nBytes, '\0', bufLen);
    // IO Read Loop
    int status = 1;
    int readIn = 0;
    do{
        status = read(fd, buffer+readIn, 1);
		// breaks and resets most recently read byte if  byte = delim
		if(buffer[readIn] == delim){
			buffer[readIn] = '\0';
			break;
		}
        readIn += status;
    } while(status > 0);
	return buffer;
}


// Reads entire file into string buffer
// Returns NULL if file does not exist, string otherwise
char* readFromFile(char* file){
    int fd = open(file, O_RDONLY);    // Returns -1 on failure, >0 on success

    // Fatal Error if file does not exist
    if(fd < 0){
        printf("Fatal Error: File does not exist.\n");
        return "FILE_DNE";
    }

    //Allocates memory for file buffer
    struct stat *buffer = (struct stat*)malloc(sizeof(struct stat));
    if(buffer == NULL){
        printf("Bad malloc\n");
        return NULL;
    }

    //Determines size of file
    stat(file, buffer);
    int buffer_size = buffer->st_size;

    // Warning: Empty file
    if(buffer_size == 0){
        printf("Warning: Empty file.\n");
		// Addresses returns stuff
		return "EMPTY_FILE";
    }

    //Mallocs and memsets file buffer for actual file contents
    char* file_buffer = (char*)malloc(buffer_size);
    if(file_buffer == NULL){
        printf("Bad malloc\n");
        return NULL;
    }
    memset(file_buffer, '\0', buffer_size);

    // IO Read Loop
    int status = 1;
    int readIn = 0;
    do{
        status = read(fd, file_buffer+readIn, buffer_size - readIn);
        readIn += status;
    } while(status > 0 && readIn < buffer_size);

    free(buffer);
    close(fd);
    return file_buffer;
}


// Given fileLL and file name, prepends new, populated file node to LL
file* addFileToLL(file* fileLL, char* name){
	char* fileString = readFromFile(name);
	// FILE DNE - not exactly sure what to do or if this should be possible
	if(strcmp(fileString, "FILE_DNE") == 0){
		return fileLL;
	}
	file* temp = (file*)malloc(sizeof(file));
	temp->next = fileLL;
	temp->fileName = name;
	temp->nameLen = strlen(name);	
	
	// EMPTY FILE - set fileLen to 0, fileData = NULL
	if(strcmp(fileString, "EMPTY_FILE") == 0){
		temp->fileData = NULL;
		temp->fileLen = 0;
	}
	// NON-EMPTY FILE - set data and len as normal
	else{
		temp->fileLen = strlen(temp->fileData);
		temp->fileData = fileString;
	}
	return temp;
}


// Given server/client socket and fileLL to send over, composes string message and writes message to fd
void transferOver(int sockfd, file* fileLL, char* command){
	if(fileLL == NULL){
		printf("Trying to transfer empty LL\n");
		return;
	}
	file* ptr = fileLL;
	int bufLen = 20;
	int numFiles = 0;
	// Calculates # files to be transferred
	while(ptr != NULL){
		numFiles++;
		ptr = ptr->next;
	}
	// <command>:<numFiles>:
	writeLoop(sockfd, command, strlen(command));
	write(sockfd, ":", 1);
	char* numFilesString = intToString(numFiles);
	writeLoop(sockfd, numFilesString, strlen(numFilesString));
	write(sockfd, ":", 1);

	// Reset loop to write each files' info to socket
	// <nameLen>:<fileName><fileLen>:<fileData>
	ptr = fileLL;
	while(ptr != NULL){
		
		char* nameLenString = intToString(ptr->nameLen);
		writeLoop(sockfd, nameLenString, strlen(nameLenString));
		write(sockfd, ":", 1);

		writeLoop(sockfd, ptr->fileName, ptr->nameLen);

		char* fileLenString = intToString(ptr->fileLen);
		writeLoop(sockfd, fileLenString, strlen(fileLenString));
		write(sockfd, ":", 1);

		writeLoop(sockfd, ptr->fileData, ptr->fileLen);
	}
}



// Loops write
void writeLoop(int fd, char* str, int numBytes){
	int strLen = strlen(str);
    // IO Read Loop
    int status = 1;
    int readIn = 0;
    do{
        status = write(fd, str + readIn, numBytes - readIn);
        readIn += status;
    } while(status > 0 && readIn < numBytes);
}

// Converts given int to string of appropriate length stored on heap
char* intToString(int num){
	char* itoabuf = (char*)malloc( (int)((ceil(log10(num))+1)) * sizeof(char));
    sprintf(itoabuf,"%d", num);
    return itoabuf;
}