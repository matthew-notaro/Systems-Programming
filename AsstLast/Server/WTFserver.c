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
#include <openssl/sha.h>
#include <dirent.h>


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
file *addDirToLL(file* fileLL, char *proj);
file* addFileToLL(file* fileLL, char* name);
void destroyProj(char* name);

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;


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
// Plain function used to test connecting
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

// Function to be made into a thread for each connection to server
// Function to be used
void* socketThread2(void* sockvoidstar){
	printf("Entering thread\n");
	int sock = *((int*)sockvoidstar);
	int n;
	// Reads desired command from socket
	char* command = readUntilDelim(sock, ':');
	char* error;
	file* fileLL = NULL;

	// CHECKOUT - Sends back entire project
	if(strcmp("checkout", command) == 0){
		char* project = readUntilDelim(sock, ':');
		fileLL = addDirToLL(fileLL, project);
		// Transfer over or error if DNE
		if(fileLL != NULL){
			transferOver(sock, fileLL, command);
		}
		else{
			error = "error:project does not exist"
			writeLoop(sock, error, strlen(error));
		}
	}

	// Sends back .Manifest of given project
	// Assumes request formatted as: <command>:<project>
	else if(strcmp("update", command) == 0 || strcmp("commit", command) == 0){
		// Format desired file name to <project>/.Manifest
		char* project = readUntilDelim(sock, ':');
		char* manWithProj = (char*)malloc( (strlen(project)+10) * sizeof(char));
    	strcpy(manWithProj, project);
    	strcat(manWithProj, "./Manifest");

		// Transfers file if exists
		fileLL = addFileToLL(fileLL, manWithProj);
		if(fileLL != NULL){
			transferOver(sock, fileLL, command);
		}
		// Writes error to socket
		else{
			error = "error:project does not exist"
			writeLoop(sock, error, strlen(error));
		}
		// Frees heap strings used
		free(project);
		free(manWithProj);
	}
	
	// Reads in names of n files to be sent back to client
	// Protocol: <cmd>:<num>:<nameLen>:<fileName>...
	else if(strcmp("upgrade", command) == 0){
		int numFiles = atoi(readUntilDelim(sock, ':'));
		int i, nameLen;
		char* fileName, *nameLenString;
		// Reads nameLen to get fileName
		for(i = 0; i < numFiles; i++){
			nameLenString = readUntilDelim(sock, ':');
			nameLen = atoi(nameLenString);
			fileName = readNBytes(sock, nameLen);
			free(nameLenString);
			fileLL = addFileToLL(fileLL, fileName);

		}
		transferOver(sock, fileLL, command);
	}

	else if(strcmp("commit", command) == 0){
		
	}
	else if(strcmp("push", command) == 0){
		
	}
	
	// CREATE - creates new project's dir and .Manifest
	// Sends "success" or "error"
	else if(strcmp("create", command) == 0){
		char* project = readUntilDelim(sock, ':');
		// Attempts to make new project's dir
		if(mkdir(project, 00600) == -1){
			error = "error:project already exists";
			writeLoop(sock, error, strlen(error));
		}
		else{
			// Creates empty .Manifest
			char* manWithProj = (char*)malloc( (strlen(project)+10) * sizeof(char));
			strcpy(manWithProj, project);
			strcat(manWithProj, "./Manifest");
			int manfd = open(manWithProj, O_RDWR|O_CREAT|O_APPEND, 00600);
			// Send success to client if creates project's .Manifest
			if(manfd > 0){
				error = "success";
				writeLoop(sock, error, strlen(error));
			}
			free(manWithProj);
		}
		free(project);
	}

	// Reads project name then recursively deletes files then the dir
	else if(strcmp("destroy", command) == 0){
		char* project = readUntilDelim(sock, ':');
		// Initial test to see if can open project dir
		DIR* currentDir = opendir(project);
		if(currentDir == NULL){
			error = "error:project does not exist"
			writeLoop(sock, error, strlen(error));
		}
		else{
			// If project exists, delete contents of project and project dir
			// Send success message to client
			destroyProj(project);
			remove(project);
			error = "success";
			writeLoop(sock, error, strlen(error));
			closedir(currentDir);
		}
		free(project);
	}

	// CURRENTVERSION - sends over all files w/ version numbers for a given project
	else if(strcmp("currentversion", command) == 0){
		char* project = readUntilDelim(sock, ':');
		char* manWithProj = (char*)malloc( (strlen(project)+10) * sizeof(char));
    	strcpy(manWithProj, project);
    	strcat(manWithProj, "./Manifest");
		char* manString = readFromFile(manWithProj);
		
	}
	else if(strcmp("history", command)){
		
	}
	else if(strcmp("rollback", command)){
		
	}
	else{
		// Bad stuff
	}
	
	free(command);

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
	if(status < 0) //File does not exist
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



// Recursively deletes all files, and subdirs in proj
// Must remove the dir that destroyProj is called on
void destroyProj(char* name){
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
			if(remove(buff) == 0){
				printf("removed file: %s.\n", buffer);
			}
		}
		else if(currentThing->d_type == DT_DIR){
			destroyProj(buff);
			// Removes empty dir
			if(remove(buff) == 0){
				printf("removed dir: %s.\n", buffer);
			}
		}
	}
	closedir(currentDir);
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
// Delim is read then overwritten so next read will start after delim
// Returns string of at most bufLen bytes stored on the heap
// Max string len set to 100 - only reads numbers, names, lines of .Manifest
char* readUntilDelim(int fd, char delim){
	// Allocate 100 bytes - should be enough to store any file name or line of .Manifest
	int bufLen = 100;
	char* buffer = (char*)malloc(bufLen*sizeof(char));
	if(buffer == NULL){
        printf("Bad malloc\n");
        return NULL;
    }
	memset(buffer, '\0', bufLen);
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


// Given project and fileLL, recursively adds each file to the fileLL
file *addDirToLL(file* fileLL, char *proj){
	DIR *currentDir = opendir(proj);
	if (currentDir == NULL)	{
		printf("ERROR: Project does not exist\n");
		return NULL;
	}
	struct dirent *currentThing = NULL;
	readdir(currentDir);
	readdir(currentDir);
	while ((currentThing = readdir(currentDir)) != NULL){
		char buff[1024];
		snprintf(buff, sizeof(buff), "%s/%s", proj, currentThing->d_name);
		if (currentThing->d_type == DT_REG){
			// Might cause memory issues since can recurse into subdirs before adding that dir's files
			fileLL = addFileToLL(fileLL, buff);
		}
		else if (currentThing->d_type == DT_DIR){
			recursion(buff);
		}
	}
	closedir(currentDir);
	return fileLL;
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