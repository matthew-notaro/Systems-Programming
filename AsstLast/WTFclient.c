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
#include <openssl/sha.h>

//#define PORT 42069

char* HOST = NULL;
char* PORT = NULL;

typedef struct file{
		char* fileName;
		char* nameLen;
		char* numBytes;
		char* fileData;
} file;

int configure(char* IPAddress, char* portNum);
int checkout(char* project);
int update(char* project);
int upgrade(char* project);
int commit(char* project);
int push(char* project);
int create(char* project);
int destroy(char* project);
int add(char* project, char* file);
int remove_(char* project, char* file);
int update(char* project);
int currentversion(char* project);
int history(char* project);
int rollback(char* project, char* version);

int setServerDetails();

int connectToServer();
char* composeMessage(char* command, file* arr, char* numFiles);
int sendToServer();

char* readFromFile(char* file);

char* hash(char* data);

char* getHashFromLine(char* line);
char* getPathFromLine(char* line);
char* getVersionFromLine(char* line);

char* readUntilDelim(int fd, char delim);

int main(int argc, char **argv) 
{
	/*if(argc < 3){
		printf("ERROR: Not enough parameters\n");
		return -1;
	}
	char* op = argv[1];
	if(strcmp(op, "configure") == 0 && argc == 4){
		if(configure(argv[2], argv[3]) == 0){
			printf("Configured\n");
			return 0;
		}
		return -1;
	}
	setServerDetails();
	// printf("host: %s\n", HOST);
	// printf("port: %s\n", PORT);
	connectToServer();
	if(strcmp(op, "checkout") == 0 && argc == 3){
		if(checkout(argv[2]) == 0){
			printf("Checkout'ed\n");
			return 0;
		}
		return -1;
	}
	else if(strcmp(op, "update") == 0 && argc == 3){
		if(update_(argv[2]) == 0){
			printf("Updated\n");
			return 0;
		}
		return -1;
	}
	else if(strcmp(op, "upgrade") == 0 && argc == 3){
		if(upgrade(argv[2]) == 0){
			printf("Upgraded\n");
			return 0;
		}
		return -1;
	}
	else if(strcmp(op, "commit") == 0 && argc == 3){
		if(upgrade(argv[2]) == 0){
			printf("Committed\n");
			return 0;
		}
		return -1;
	}
	else if(strcmp(op, "push") == 0 && argc == 3){
		if(upgrade(argv[2]) == 0){
			printf("Pushed\n");
			return 0;
		}
		return -1;
	}
	else if(strcmp(op, "create") == 0 && argc == 3){
		if(create(argv[2]) == 0){
			printf("Created\n");
			return 0;
		}
		return -1;
	}
	else if(strcmp(op, "destroy") == 0 && argc == 3){
		if(destroy(argv[2]) == 0){
			printf("Destroyed\n");
			return 0;
		}
		return -1;
	}
	else if(strcmp(op, "add") == 0 && argc == 4){
		if(add(argv[2], argv[3]) == 0){
			printf("Added\n");
			return 0;
		}
		return -1;
	}
	else if(strcmp(op, "remove") == 0 && argc == 4){
		if(remove_(argv[2], argv[3]) == 0){
			printf("Removed\n");
			return 0;
		}
		return -1;
	}
	else if(strcmp(op, "currentversion") == 0 && argc == 3){
		if(currentversion(argv[2]) == 0){
			printf("Current versioned\n");
			return 0;
		}
		return -1;
	}
	else if(strcmp(op, "history") == 0 && argc == 3){
		if(history(argv[2]) == 0){
			printf("History'ed\n");
			return 0;
		}
		return -1;
	}
	else if(strcmp(op, "rollback") == 0 && argc == 4){
		if(rollback(argv[2], argv[3]) == 0){
			printf("Rolled back\n");
			return 0;
		}
		return -1;
	}
	else{
		printf("ERROR: Invalid command\n");
		return -1;
	}*/
	
	
	//int sockfd = connectToServer();
	char* command = malloc(5);
	command = "check";
	char* numFiles = malloc(1);
	numFiles = "2";
	struct file arr[2];
	arr[0].fileName = "file1"; 
	arr[0].nameLen = "5"; 
	arr[0].numBytes = "9"; 
	arr[0].fileData = "file1data"; 
	arr[1].fileName = "file2"; 
	arr[1].nameLen = "5"; 
	arr[1].numBytes = "9"; 
	arr[1].fileData = "file2data"; 
	
	update(command);
	
	//sendToServer(sockfd, command);
	//composeMessage(command, arr, numFiles);
	//getHash(command);
	
	return 0;
}

int configure(char* IPAddress, char* portNum)
{
	int IPAddressSize = 0, portNumSize = 0, bufferSize = 0;
	char* configName = ".configure";
	char* space = malloc(sizeof(char)*1);
	space = " ";

	int fd = open(configName, O_RDWR|O_CREAT|O_APPEND, 00600);
	
  if(fd < 0)
	{
    printf("File does not exist.\n"); 
    return -1;
  }
	
	IPAddressSize = strlen(IPAddress) + 1;
	portNumSize = strlen(portNum) + 1;
	bufferSize = IPAddressSize+portNumSize+sizeof(char)*1;
	
	char* buffer = malloc(bufferSize);
	
	strcpy(buffer, IPAddress);
	strcat(buffer, space);
	strcat(buffer, portNum);
	
	printf("buffer: %s\n", buffer);
	
	write(fd, buffer, strlen(buffer));

	return 0;
}

int checkout(char* project)
{
	return 0;
}

int update(char* project)
{
	//failure cases
	//char* command = malloc();
	//char* message = composeMessage(); //get manifest
	
	// TESTING 
	
	char* clientManifest = malloc(2);
	char* serverManifest = malloc(2);
	clientManifest = ".cl";
	serverManifest = ".s";
	
	//Opens both client and server .Manifest files
	int cliManifestFd = open(clientManifest, O_RDONLY);
	int servManifestFd = open(serverManifest, O_RDONLY); //Only used to compare versions
	
	//Extracts version numbers
	char* cManifestVer = readUntilDelim(cliManifestFd, '\n');
	char* sManifestVer = readUntilDelim(servManifestFd, '\n');
	
	printf("versions: %s %s\n", cManifestVer, sManifestVer);
	
	close(servManifestFd);
	//Compares version numbers
	
	int updateFd = open(".Update", O_RDWR|O_CREAT|O_APPEND, 00600);
	
	if(strcmp(cManifestVer, sManifestVer) == 0) //Case 1: same Manifest versions
	{
		//delete conflict if it exists
		//"up to date" to stdout
		return 0;
	}

	//Case 2: different Manifest versions
	//Loops through each line of client Manifest
	while(1)
	{
		//Extracts line from client .Manifest and gets the project path
		char* currentClientLine = readUntilDelim(cliManifestFd, '\n');
		printf("ccl: %s\n", currentClientLine);
		
		//Closes fd and exits loop if no more lines
		if(currentClientLine == NULL || strlen(currentClientLine) == 0)
		{
			close(cliManifestFd);
			break;
		}
		
		//Extracts project path from line
		char* clientProjPath = getPathFromLine(currentClientLine);
				
		//Opens server Manifest and skips over version number
		servManifestFd = open(serverManifest, O_RDONLY);
		readUntilDelim(servManifestFd, '\n'); 
		
		//Loops through each line of server Manifest to check if project exists
		while(1)
		{
			//Extracts line from server .Manifest and gets the project path
			char* currentServerLine = readUntilDelim(servManifestFd, '\n');
			
			//Case 2.1: project not found on server side
			if(currentServerLine == NULL || strlen(currentServerLine) == 0)
			{
				//must delete code
				close(servManifestFd);
				break;
			}
			
			//Extracts project from line
			char* serverProjPath = getPathFromLine(currentServerLine);
			
			//Case 2.2: project found on server side
			if(strcmp(clientProjPath,serverProjPath) == 0)
			{
				char* cliVersion = getVersionFromLine(currentClientLine);
				char* servVersion = getVersionFromLine(currentServerLine);
				
				char* storedCliHash = getHashFromLine(currentClientLine);
				char* servHash = getHashFromLine(currentServerLine);
				
				//Checks if versions don't match
				if(strcmp(cliVersion,servVersion) != 0)
				{
					//Checks if stored and server hashes don't match
					if(strcmp(storedCliHash,servHash) != 0)
					{
						char* currentProjString = readFromFile(clientProjPath);
						char* liveHash = hash(currentProjString);
						
						if(strcmp(storedCliHash,servHash) == 0)
						{
							//Case 2.1.1: server has modified files
							//append to update
							//output
							free(currentProjString);
							free(liveHash);
							close(servManifestFd);
							break;
						}
						else
						{
							//Case 2.1.2: server has removed files
							//append to conflict
							//stdout
							free(currentProjString);
							free(liveHash);
							close(servManifestFd);
							break;
						}
					}
				}
				free(cliVersion);
				free(servVersion);
				free(storedCliHash);
				free(servHash);
			}
			free(currentServerLine);
			free(serverProjPath);
		}
		free(currentClientLine);
		free(clientProjPath);
	}
	close(cliManifestFd);
	
	//Open server manifest again
	servManifestFd = open(serverManifest, O_RDONLY);
	
	//Loop through server .Manifest
	while(1)
	{
		char* currentServerLine = readUntilDelim(servManifestFd, '\n');
		
		//Closes fd and exits loop if no more lines
		if(currentServerLine == NULL || strlen(currentServerLine) == 0)
		{
			close(servManifestFd);
			break;
		}
		
		char* serverProjPath = getPathFromLine(currentServerLine);
		
		//Opens client Manifest and skips over version number
		cliManifestFd = open(clientManifest, O_RDONLY);
		readUntilDelim(cliManifestFd, '\n');
		
		while(1)
		{
			char* currentClientLine = readUntilDelim(cliManifestFd, '\n');
			
			//Case 2.3: server has files not on client side
			if(currentClientLine == NULL || strlen(currentClientLine) == 0)
			{
				//must delete code
				close(cliManifestFd);
				break;
			}
			
			char* clientProjPath = getPathFromLine(currentServerLine);
			
			//if project found on client side
			if(strcmp(clientProjPath,serverProjPath) == 0)
			{
				close(cliManifestFd);
				break;
			}
			
			free(currentClientLine);
			free(clientProjPath);
		} 
		free(currentServerLine);
		free(serverProjPath);
	}
	close(servManifestFd);
	
	return 0;
}

int upgrade(char* project)
{
	//fail if proj dne on server
	//fail if no cxn
	//fail if no .update_
	//fail if .conflict
	
	
	
	return 0;
}

int commit(char* project)
{
	//char* message = composeMessage(); //client send server commit <proj>
	//receive .manifest
	//client fails if project DNE on server
	// client fails if server cannot be contacted
	// client fails if it cannot fetch the server's .manifest file for the project,
	
	int status; 
  status = open(".Update", O_RDONLY);
	if(status >= 0)
	{
		//CHECK IF EMPTY
		printf("Error: .Update exists.");
		return -1;
	}
	status = open(".Conflict", O_RDONLY);
	if(status >= 0)
	{
		//CHECK IF EMPTY
		printf("Error: .Conflict exists.");
		return -1;
	}
	
	char* clientManifest;
	char* serverManifest;
	
	int cliManifestFd = open(clientManifest, O_RDONLY); //WRITE APPEND
	int servManifestFd = open(serverManifest, O_RDONLY);
	
	char* cManifestVer = readUntilDelim(cliManifestFd, '\n');
	char* sManifestVer = readUntilDelim(servManifestFd, '\n');
	
	if(strcmp(cManifestVer, sManifestVer) != 0) //Manifest versions differ
	{
		//error, stop, ask for update
	}
	
	while(1)
	{
		char* currentLine = readUntilDelim(cliManifestFd, '\n');
		if(currentLine == NULL || strlen(currentLine) == 0)
		{
			break;
		}
		
		char* storedHash = getHashFromLine(currentLine);
		char* currProjPath = getPathFromLine(currentLine);
 		char* currProjString = readFromFile(currProjPath);
		char* liveHash = getHashFromLine(currProjString);
				
		if(strcmp(storedHash, liveHash) != 0)
		{
			int cliManifestFd = open("project/.commit", O_RDONLY); 
		}
	}

	return 0;
}

int push(char* project)
{
	return 0;
}

int create(char* project)
{
	char* message;
	int sockfd = connectToServer();
	int status;
	
	sendToServer(sockfd, message);
  status = mkdir(project, 00600); 
	if(status < 0)
	{
		
	}
	//struct file** arr = malloc(numFiles*sizeof(struct file*))

	
	//get manifest from server
	return 0;
}

int destroy(char* project)
{
	return 0;
}

int add(char* project, char* file)
{
	int status; 
  status = open(file, O_RDONLY);
	if(status < 0)
	{
		printf("Error: file does not exist.");
		return -1;
	}
	// open manifest
	// append write
	
	return 0;
}

int remove_(char* project, char* file)
{
	int status; 
  status = open(file, O_RDONLY);
	if(status < 0)
	{
		printf("Error: file does not exist.");
		return -1;
	}
	else
	{
		// open manifest
		// find line
		// load data into buffer
		// delete manifest contents
		// write
		// write
	}
	
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
	//CHECK THIS
	//doesn't work for me unless i myself go into .configure and edit it...
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

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
	if(sockfd < 0)
	{
		printf("ERROR\n");
		return -1;
	}
	struct hostent* host = gethostbyname("127.0.0.1");
	struct sockaddr_in serverAddressInfo;
	
	bzero((char*)&serverAddressInfo, sizeof(serverAddressInfo));
	
	serverAddressInfo.sin_family = AF_INET;
	serverAddressInfo.sin_addr.s_addr = htons(INADDR_ANY);
	serverAddressInfo.sin_port = htons(42069); //CHANGE BACK TO PORTNO
	
	bcopy((char*)host->h_addr, (char*)&serverAddressInfo.sin_addr.s_addr, host->h_length);
	
	cxn_status = connect(sockfd, (struct sockaddr *)&serverAddressInfo, sizeof(serverAddressInfo));
	if(cxn_status < 0)
	{
		printf("ERROR\n");
		return -1;
	}
	
	return sockfd;
}

//Composes a message based on delimiter-based protocol
char* composeMessage(char* command, file* arr, char* numFiles)
{
	int commandLen = strlen(command);
	int numFilesLen = 1;
	int sizeOfBuffer = commandLen + numFilesLen + 2; // +2 to account for delimiters
	char* delim = malloc(sizeof(char)*1);
	delim = ":";
	
	int i = 0;
	
	for(i = 0; i < atoi(numFiles); i++)
	{
	  file* curr = &arr[i];
	 	sizeOfBuffer += strlen(curr->nameLen); //len of nameLen
	 	sizeOfBuffer += strlen(curr->fileName); //len of fileName 
	 	sizeOfBuffer += strlen(curr->numBytes); //len of numBytes
	 	sizeOfBuffer += atoi(curr->numBytes); //numBytes
	 	sizeOfBuffer += 2; //to account for delimiters
	}
	
	char* buffer = malloc(sizeOfBuffer);
	strcpy(buffer, command);
	strcat(buffer, delim);
	strcat(buffer, numFiles);
	strcat(buffer, delim);
	
	for(i = 0; i < atoi(numFiles); i++)
	{
	  file* curr = &arr[i];
	  strcat(buffer, curr->nameLen);
  	strcat(buffer, delim);
	 	strcat(buffer, curr->fileName);
	 	strcat(buffer, curr->numBytes);
	 	strcat(buffer, delim);
		strcat(buffer, curr->fileData);
  }
	
	printf("buffer: %s\n", buffer);
  return buffer;
	
}

int sendToServer(int sockfd, char* message)
{
	char* buffer[256]; //check type
	int n = 0;
	bzero(buffer,256);
	
	n = write(sockfd,message,strlen(message));
	if (n < 0) 
		 printf("ERROR writing to socket\n");
		 
	n = read(sockfd,buffer,255);
	 if (n < 0) 
			printf("ERROR reading from socket\n");
	
	//printf("Message: %s\n", buffer);
	
	return 0;
}

// Reads entire file into string buffer
// Returns NULL if file does not exist, string otherwise
char* readFromFile(char* file)
{
    int fd = open(file, O_RDONLY);    // Returns -1 on failure, >0 on success
    // Fatal Error if file does not exist
    if(fd < 0){
        printf("Fatal Error: File does not exist.\n");
        return "FILE_DNE";
    }
    struct stat *buffer = malloc(sizeof(struct stat));
    if(buffer == NULL){
        printf("Bad malloc\n");
        return NULL;
    }
    stat(file, buffer);
    int bufferSize = buffer->st_size;
    // Warning: Empty file
    if(bufferSize == 0){
        printf("Warning: Empty file.\n");
				return "EMPTY_FILE";
    }
		
    // IO Read Loop
    char* fileBuffer = (char*)malloc(bufferSize);
    if(fileBuffer == NULL){
        printf("Bad malloc\n");
        return NULL;
    }
    memset(fileBuffer, '\0', bufferSize);
    int status = 1;
    int readIn = 0;
    do{
        status = read(fd, fileBuffer+readIn, bufferSize - readIn);
        readIn += status;
    } while(status > 0 && readIn < bufferSize);

    free(buffer);
    return fileBuffer;
}

// Hashes a given string and returns the code 
char* hash(char* data)
{
	int x = 0;
	size_t length = strlen(data);
	char* buffer = malloc(SHA_DIGEST_LENGTH);
	unsigned char hash[SHA_DIGEST_LENGTH];
	//SHA1(data, length, hash);
	
	for(x = 0; x < SHA256_DIGEST_LENGTH; x++)
	{
		sprintf(buffer+(x * 2), "%02x", hash[x]);
	}
	
	printf("hash code: %s\n", buffer);
	return buffer;
}

// Extracts stored hash code from .Manifest line
// Returns hash string
char* getHashFromLine(char* line)
{
	int i = 0, start = 0, end = 0, len = strlen(line);
	
	end = strlen(line)-1;
	for(i = end-1; i > 0; i--)
	{
		if(isspace(line[i]) != 0)
		{
			start = i+1;
			break;
		}
	}
	
	int bufSize = end-start+1;
	char* buffer = malloc(end-start+1);
	
	for(i = 0; i < bufSize; i++)
	{
		buffer[i] = line[i+start];
	}

	return buffer;
}

// Extracts project path from .Manifest line
// Returns path string
char* getPathFromLine(char* line)
{
	int i = 0, start = 0, end = 0, len = strlen(line);
	
	for(i = 0; i < len; i++)
	{
		if(line[i] == ' ' || line[i] == '\t') //space found
		{
			if(start == 0)
				start = i+1;
			else
				end = i-1;
		}
	}

	int bufSize = end-start+1;
	char* buffer = malloc(end-start+1);
	
	for(i = 0; i < bufSize; i++)
	{
		buffer[i] = line[i+start];
	}

	return buffer;
}

// Extracts version number from .Manifest line
// Returns version string
char* getVersionFromLine(char* line)
{
	int i = 0, end = 0, len = strlen(line);
	
	for(i = 0; i < len; i++)
	{
		if(line[i] == ' ' || line[i] == '\t') //space found
		{
			end = i-1;
		}
	}

	char* buffer = malloc(end);
	
	for(i = 0; i < end; i++)
	{
		buffer[i] = line[i];
	}

	return buffer;
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

