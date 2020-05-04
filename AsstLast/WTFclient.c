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
#include <dirent.h>

char* HOST = NULL;
char* PORT = NULL;

typedef struct file{
	char* fileName;
	int nameLen;
	int fileLen;
	char* fileData;
	struct file* next;
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
//char* composeMessage(char* command, file* arr, char* numFiles);
int sendToServer();

char* hash(char* data);

char* getHashFromLine(char* line);
char* getFileFromLine(char* line);
char* getVersionFromLine(char* line);

char* readFromFile(char* file);
char* readUntilDelim(int fd, char delim);
void writeLoop(int fd, char* str, int numBytes);
char* intToString(int num);

void transferOver(int sockfd, file* fileLL, char* command);
file *addDirToLL(file* fileLL, char *proj);
file* addFileToLL(file* fileLL, char* name);

int main(int argc, char **argv) 
{
	if(argc < 3){
		printf("ERROR: Not enough parameters\n");
		return -1;
	}
	char* op = argv[1];
	if(strcmp(op, "configure") == 0 && argc == 4){
		if(configure(argv[2], argv[3]) == 0){
			printf("Configure successful.\n");
			return 0;
		}
		return -1;
	}
	setServerDetails();
	//connectToServer();
	if(strcmp(op, "checkout") == 0 && argc == 3){
		if(checkout(argv[2]) == 0){
			printf("Checkout successful\n");
			return 0;
		}
		return -1;
	}
	else if(strcmp(op, "update") == 0 && argc == 3){
		if(update(argv[2]) == 0){
			printf("Update successful.\n");
			return 0;
		}
		return -1;
	}
	else if(strcmp(op, "upgrade") == 0 && argc == 3){
		if(upgrade(argv[2]) == 0){
			printf("Upgrade successful.\n");
			return 0;
		}
		return -1;
	}
	else if(strcmp(op, "commit") == 0 && argc == 3){
		if(upgrade(argv[2]) == 0){
			printf("Commit successful.\n");
			return 0;
		}
		return -1;
	}
	else if(strcmp(op, "push") == 0 && argc == 3){
		if(upgrade(argv[2]) == 0){
			printf("Push successful.\n");
			return 0;
		}
		return -1;
	}
	else if(strcmp(op, "create") == 0 && argc == 3){
		if(create(argv[2]) == 0){
			printf("Project created.\n");
			return 0;
		}
		return -1;
	}
	else if(strcmp(op, "destroy") == 0 && argc == 3){
		if(destroy(argv[2]) == 0){
			printf("Project destroyed\n");
			return 0;
		}
		return -1;
	}
	else if(strcmp(op, "add") == 0 && argc == 4){
		if(add(argv[2], argv[3]) == 0){
			printf("File added\n");
			return 0;
		}
		return -1;
	}
	else if(strcmp(op, "remove") == 0 && argc == 4){
		if(remove_(argv[2], argv[3]) == 0){
			printf("File removed\n");
			return 0;
		}
		return -1;
	}
	else if(strcmp(op, "currentversion") == 0 && argc == 3){
		if(currentversion(argv[2]) == 0){
			printf("CurrentVersion successful\n");
			return 0;
		}
		return -1;
	}
	else if(strcmp(op, "history") == 0 && argc == 3){
		if(history(argv[2]) == 0){
			printf("History successful\n");
			return 0;
		}
		return -1;
	}
	else if(strcmp(op, "rollback") == 0 && argc == 4){
		if(rollback(argv[2], argv[3]) == 0){
			printf("Rollback successful\n");
			return 0;
		}
		return -1;
	}
	else{
		printf("ERROR: Invalid command\n");
		return -1;
	}
	
	//int sockfd = connectToServer();
	// char* command = malloc(5);
	// command = "proj1";
	// char* numFiles = malloc(1);
	// numFiles = "2";
	// struct file arr[2];
	// arr[0].fileName = "file1"; 
	// arr[0].nameLen = "5"; 
	// arr[0].numBytes = "9"; 
	// arr[0].fileData = "file1data"; 
	// arr[1].fileName = "file2"; 
	// arr[1].nameLen = "5"; 
	// arr[1].numBytes = "9"; 
	// arr[1].fileData = "file2data"; 
	
	//sendToServer(sockfd, command);
	//composeMessage(command, arr, numFiles);
	//getHash(command);
	//commit(command);
	//checkout(command);
	
	return 0;
}

int configure(char* IPAddress, char* portNum) //Tested
{
	int IPAddressSize = 0, portNumSize = 0, bufferSize = 0;
	char* configName = ".configure";
	char* space = malloc(sizeof(char)*1);
	space = " ";

	int fd = open(configName, O_RDWR|O_CREAT|O_APPEND, 0777);
	
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
	
	writeLoop(fd, buffer, strlen(buffer));

	return 0;
}

int checkout(char* project) //***
{
	//Checks that client has already configured
	int configureFd = open(".Configure", O_RDONLY);
	if(configureFd < 0)
	{
		printf("Error: Please configure.");
		return -1;
	}
	
	//Checks that project does not exist on client
	int projectFd = open(project, O_RDONLY);
	if(projectFd >= 0)
	{
		close(projectFd);
		printf("Error: Project already exists on client.");
		return -1;
	}
	
	//Composes message to send to server
	char* clientcommand = malloc(8);
	char* delim = malloc(1);
	clientcommand = "checkout";
	delim = ":";
	char* message = malloc(strlen(project)+8+1);
	strcpy(message, clientcommand);
	strcat(message, delim);
	strcat(message, project);
	
	//Connects to server and sends message
	int sockfd = connectToServer();
	if(sockfd < 0)
	{
		printf("Error: could not connect to server.\n");
		return -1;
	}
	int newsockfd = sendToServer(sockfd, message);
	
	//Reads server response 
	//On success:
	
	//Extracts information about upcoming data in from socket
	char* command = readUntilDelim(newsockfd, ':');
	char* numFilesString = readUntilDelim(newsockfd, ':');
	int numFiles = atoi(numFilesString);
	
	//Loops through file data coming from socket
	int i = 0, status = 1, readIn = 0;
	for(i = 0; i < numFiles; i++)
	{
		//Reads in file name
		char* nameLenString = readUntilDelim(newsockfd, ':');
		int nameLen = atoi(nameLenString);
		char* fileName = malloc(nameLen);
	  do
		{
	    status = read(newsockfd, fileName+readIn, nameLen - readIn);
	    readIn += status;
	  } while(status > 0 && readIn < nameLen);
		
		//Resets status and readIn
		status = 1;
	  readIn = 0;
		
		//Reads in file data
	  char* fileLenString = readUntilDelim(newsockfd, ':');
		int fileLen = atoi(fileLenString);
	  char* fileData = malloc(fileLen);
		do
		{
	    status = read(newsockfd, fileData+readIn, fileLen - readIn);
	    readIn += status;
	  } while(status > 0 && readIn < fileLen);
		
		//Resets status and readIn
		status = 1;
	  readIn = 0;
		
		//Creates file and writes data
		int fd = open(fileName, O_RDWR|O_CREAT|O_APPEND, 0777);
		writeLoop(fd, fileData, fileLen);
	}
	
	return 0;
}

int update(char* project) //***
{
	//Composes message for server
	char* clientcommand = malloc(6);
	char* delim = malloc(1);
	clientcommand = "update";
	delim = ":";
	char* message = malloc(strlen(project)+6+1);
	strcpy(message, clientcommand);
	strcat(message, delim);
	strcat(message, project);
	
	//Connects to server, sends message
	int sockfd = connectToServer();
	if(sockfd < 0)
	{
		printf("Error: could not connect to server.\n");
		return -1;
	}
	
	//Reads server response 
	//Extracts information about upcoming data in from socket
	char* numBytesString = readUntilDelim(sockfd, ':');
	if(strcmp(numBytesString, "error") == 0)
	{
		printf("Error: project does not exist.");
		return -1;
	}
	
	//On success, get number of bytes to read
	int numBytes = atoi(numBytesString);
	
	//Reads in file data
	char* manifestData = (char*)malloc(numBytes);

	int status = 1;
	int readIn = 0;
		
	do
	{
		  status = read(sockfd, manifestData+readIn, numBytes - readIn);
		  readIn += status;
	} while(status > 0 && readIn < numBytes);
	
	char* clientManifest = (char*)malloc(strlen(project)+12);
	char* slash = (char*)malloc(1);
	char* dot = (char*)malloc(1);
	char* manifest = (char*)malloc(9);
	manifest = ".Manifest";
	slash = "/";
	dot = ".";
	strcpy(clientManifest, dot);
	strcat(clientManifest, slash);
	strcat(clientManifest, project);
	strcat(clientManifest, slash);
	strcat(clientManifest, manifest);
	
	char* serverManifest = malloc(13);
	serverManifest = ".servManifest";
	
	int fd = open(serverManifest, O_RDWR|O_CREAT|O_APPEND, 0777);
	write(fd, manifestData, strlen(manifestData));
	close(fd);

	//Opens both client and server .Manifest files
	int cliManifestFd = open(clientManifest, O_RDONLY);
	int servManifestFd = open(serverManifest, O_RDONLY); //Only used to compare versions
	
	//Extracts version numbers
	char* cManifestVer = readUntilDelim(cliManifestFd, '\n');
	char* sManifestVer = readUntilDelim(servManifestFd, '\n');
	
	//Case 1: same Manifest versions
	if(strcmp(cManifestVer, sManifestVer) == 0) 
	{
		char* conflictPath = malloc(strlen(project)+12);
		char* conflictFile = (char*)malloc(9);
		conflictFile = ".Conflict";
		strcpy(conflictPath, dot);
		strcat(conflictPath, slash);
		strcat(conflictPath, project);
		strcat(conflictPath, slash);
		strcat(conflictPath, conflictFile);
		int toBeRemoved = open(conflictPath, O_RDONLY);
		if(toBeRemoved >= 0)
		{
			remove(conflictPath);
			close(toBeRemoved);
		}
		printf("Up To Date\n");
		close(cliManifestFd);
		close(servManifestFd);
		//free(cManifestVer);
		//free(sManifestVer);
		remove(".servManifest");
		return 0;
	}

	close(servManifestFd);
	//free(cManifestVer);
	//free(sManifestVer);
	
	char* updatePath = malloc(strlen(project)+10);
	char* updateFile = (char*)malloc(7);
	updateFile = ".Update";
	strcpy(updatePath, dot);
	strcat(updatePath, slash);
	strcat(updatePath, project);
	strcat(updatePath, slash);
	strcat(updatePath, updateFile);
	
	int updateFd = open(updatePath, O_RDWR|O_CREAT|O_APPEND, 0777);
	char* currentClientLine = NULL, *clientFile = NULL, *cliVersion = NULL, *storedCliHash = NULL;

	//Case 2: different Manifest versions
	//Loops through each line of client Manifest
	while(1)
	{
		printf("checkpoint -1\n");
		//Extracts line from client .Manifest
		currentClientLine = readUntilDelim(cliManifestFd, '\n');
		//printf("curr cli line: %s\n", currentClientLine);
		
		//Closes fd and exits loop if no more lines
		if(currentClientLine == NULL || strlen(currentClientLine) == 0)
		{
			memset(currentClientLine, '\0', strlen(currentClientLine));
			//free(currentClientLine);
			close(cliManifestFd);
			break;
		}
		
		
		//Extracts client details
		clientFile = getFileFromLine(currentClientLine);
		cliVersion = getVersionFromLine(currentClientLine);
		//storedCliHash = malloc(strlen(currentClientLine));
		storedCliHash = getHashFromLine(currentClientLine);
	
		//Opens server Manifest and skips over version number
		servManifestFd = open(serverManifest, O_RDONLY);
		readUntilDelim(servManifestFd, '\n'); 
		
		// char* currentServerLine = NULL, *serverFile = NULL, *servVersion = NULL, *servHash = NULL;
		// 
		// currentServerLine = readUntilDelim(servManifestFd, '\n');
		// //Extracts details
		// serverFile = getFileFromLine(currentServerLine);
		// servVersion = getVersionFromLine(currentServerLine);
		// //servHash = malloc(strlen(currentServerLine));
		// servHash = getHashFromLine(currentServerLine);
	
		// printf("serv hash: %s\n", servHash);
		// printf("stored hash: %s\n", storedCliHash);
		// printf("strcmp result %d\n", strcmp(storedCliHash,servHash));
		
		
		//Loops through each line of server Manifest to check if file exists
		while(1)
		{
			//Extracts line from server .Manifest
			char* currentServerLine = NULL, *serverFile = NULL, *servVersion = NULL, *servHash = NULL;
			
			currentServerLine = readUntilDelim(servManifestFd, '\n');
			
			//Case 2.1: server has removed files from the project
			if(currentServerLine == NULL || strlen(currentServerLine) == 0)
			{
				//char* storedCliHash = getHashFromLine(currentClientLine);
				char action[256]; //+4 to account spaces and actionCode
				char* actionCode = malloc(sizeof(char)*2);
				char* space = malloc(sizeof(char)*1);
				char* newLine = malloc(sizeof(char)*1);
				actionCode = "D ";
				space = " ";
				newLine = "\n";
				
				strcpy(action, actionCode);
				strcat(action, clientFile);
				
				printf("%s\n", action);
				
				strcat(action, space);
				strcat(action, storedCliHash);
				strcat(action, newLine);
				
				writeLoop(updateFd, action, strlen(action));
				printf("written action: %s\n", action);
				
				memset(action, '\0', strlen(action));
				memset(storedCliHash, '\0', strlen(storedCliHash));
				//free(storedCliHash);
				//free(action);
				close(servManifestFd);
				break;
			}
			printf("checkpoint 0\n");
			//Extracts details
			serverFile = getFileFromLine(currentServerLine);
			servVersion = getVersionFromLine(currentServerLine);
			//servHash = malloc(strlen(currentServerLine));
			servHash = getHashFromLine(currentServerLine);
			//printf("serv hash: %s\n", servHash);
		
			//Case 2.2: file found on server side
			if(strcmp(clientFile,serverFile) == 0)
			{
				printf("checkpoint 1: file found on server\n");

				//Checks if versions don't match
				if(strcmp(cliVersion,servVersion) != 0)
				{
					printf("checkpoint 2: versions dont match\n");
					
					//memset(servHash, '\0', strlen(servHash));
					printf("serv hash: %s\n", servHash);
					//printf("stored hash: %s\n", storedCliHash);
					
					printf("strcmp result %d\n", strcmp(storedCliHash,servHash));

					//Checks if stored and server hashes don't match
					if(strcmp(storedCliHash,servHash) != 0)
					{
						printf("checkpoint 3: stored and server dont match\n");

						char* filePath = malloc(strlen(project)+strlen(clientFile)+1);// proj1/example1.txt
						char* slash = malloc(1);
						slash = "/";
						strcpy(filePath, project);
						strcat(filePath, slash);
						strcat(filePath, clientFile);
						
						printf("filePath: %s\n", filePath);
						
						char* currentFileString = readFromFile(filePath);
						char* liveHash = hash(currentFileString);
		
						printf("live hash: %s\n", liveHash);
						//printf("strlen(liveHash) = %d\n", strlen(liveHash));
						printf("stored hash: %s\n", storedCliHash);
						
						if(strcmp(storedCliHash,liveHash) == 0)
						{
							
							printf("checkpoint 4: stored and live match\n");
							//Case 2.2.1: server has modified files
							char action[256]; //malloc(strlen(clientFile)+liveHashSize+sizeof(char)*4); //+3 to account spaces and actionCode
							char* actionCode = malloc(sizeof(char)*2);
							char* space = malloc(sizeof(char)*1);
							char* newLine = malloc(sizeof(char)*1);
							actionCode = "M ";
							space = " ";
							newLine = "\n";
							
							printf("serv hash: %s\n", servHash);
							
							strcpy(action, actionCode);
							strcat(action, clientFile);
							printf("%s\n", action);
							strcat(action, space);
							strcat(action, servHash);
							strcat(action, newLine);
							writeLoop(updateFd, action, strlen(action));
							printf("written action: %s\n", action);

							/*
							memset(currentServerLine, '\0', strlen(currentServerLine));
							memset(serverFile, '\0', strlen(serverFile));
							memset(cliVersion, '\0', strlen(cliVersion));
							memset(servVersion, '\0', strlen(servVersion));
							memset(storedCliHash, '\0', strlen(storedCliHash));
							memset(servHash, '\0', strlen(servHash));
							memset(currentFileString, '\0', strlen(currentFileString));
							memset(liveHash, '\0', strlen(liveHash));
							memset(action, '\0', strlen(action));
							
							//free(currentServerLine);
							//free(serverFile);
							//free(cliVersion);
							//free(servVersion);
							//free(storedCliHash);
							//free(servHash);
							//free(currentFileString);
							//free(liveHash);
							//free(action);
							*/
							
							close(servManifestFd);
							break;
							
						}
						else
						{
							//Case 2.1.2: files conflict
							char action[256]; //malloc(strlen(clientFile)+liveHashSize+sizeof(char)*4); //+3 to account spaces and actionCode
							char* actionCode = malloc(sizeof(char)*2);
							char* space = malloc(sizeof(char)*1);
							char* newLine = malloc(sizeof(char)*1);
							char* liveHashCopy = malloc(strlen(liveHash));
							liveHashCopy = liveHash;
							actionCode = "C ";
							space = " ";
							newLine = "\n";
							
							strcpy(action, actionCode);
							strcat(action, clientFile);
						  printf("%s\n", action);
							strcat(action, space);
							strcat(action, liveHashCopy);
							strcat(action, newLine);				
							
							char* conflictPath = malloc(strlen(project)+10);
							char* conflictFile = (char*)malloc(10);
							conflictFile = ".Conflict";
							
							strcpy(conflictPath, dot);
							strcat(conflictPath, slash);
							strcat(conflictPath, project);
							strcat(conflictPath, slash);
							strcat(conflictPath, conflictFile);			
							
							int conflictFd = open(conflictPath, O_RDONLY);
							if(conflictFd < 0)
							{
								conflictFd = open(conflictPath, O_RDWR|O_CREAT|O_APPEND, 0777);
						  }

							writeLoop(conflictFd, action, 256);
							
							// memset(currentServerLine, '\0', strlen(currentServerLine));
							// memset(serverFile, '\0', strlen(serverFile));
							// memset(cliVersion, '\0', strlen(cliVersion));
							// memset(servVersion, '\0', strlen(servVersion));
							// memset(storedCliHash, '\0', strlen(storedCliHash));
							// memset(servHash, '\0', strlen(servHash));
							// memset(currentFileString, '\0', strlen(currentFileString));
							// memset(liveHash, '\0', strlen(liveHash));
							// memset(action, '\0', strlen(action));
							
							// free(currentServerLine);
						  // free(serverFile);
							// free(cliVersion);
							//free(servVersion);
							// free(storedCliHash);
							// free(servHash);
							// free(currentFileString);
							// free(liveHash);
							
							close(servManifestFd);
							close(conflictFd);
							break;
						}	
					}
				
				}
				break;
			}
		}
	}
	close(cliManifestFd);
	
	//Open server manifest again, skip over version
	servManifestFd = open(serverManifest, O_RDONLY);
	readUntilDelim(servManifestFd, '\n');
	
	//Loop through server .Manifest
	while(1)
	{
		printf("second loop checkpoint\n");
		char* currentServerLine = readUntilDelim(servManifestFd, '\n');
		
		//Closes fd and exits loop if no more lines
		if(currentServerLine == NULL || strlen(currentServerLine) == 0)
		{
			close(servManifestFd);
			break;
		}
		
		char* serverFile = getFileFromLine(currentServerLine);
		char* servHash = getHashFromLine(currentServerLine);
		
		//Opens client Manifest and skips over version number
		cliManifestFd = open(clientManifest, O_RDONLY);
		readUntilDelim(cliManifestFd, '\n');
		
		while(1)
		{
			printf("second loop inner loop checkpoint\n");
			char* currentClientLine = readUntilDelim(cliManifestFd, '\n');
			
			printf("currentClientLine: %s\n", currentClientLine);
			
			//Case 2.3: server has files not on client side
			if(currentClientLine == NULL || strlen(currentClientLine) == 0)
			{
				printf("server has files not on client side\n");

				char action[256]; //malloc(strlen(clientFile)+liveHashSize+sizeof(char)*4); //+3 to account spaces and actionCode
				char* actionCode = malloc(sizeof(char)*2);
				char* space = malloc(sizeof(char)*1);
				char* newLine = malloc(sizeof(char)*1);
				actionCode = "A ";
				space = " ";
				newLine = "\n";
								
				strcpy(action, actionCode);
				strcat(action, serverFile);
				printf("%s\n", action);
				strcat(action, space);
				strcat(action, servHash);
				strcat(action, newLine);
				writeLoop(updateFd, action, strlen(action));
				printf("written action: %s\n", action);
				
			//	memset(action, '\0', strlen(action));
				//memset(currentClientLine, '\0', strlen(currentClientLine));
				//memset(serverFile, '\0', strlen(serverFile));
				//memset(servHash, '\0', strlen(servHash));
				
				//free(action);
				//free(currentClientLine);
				////free(serverFile);
				////free(servHash);
				
				close(servManifestFd);
				break;
			}
			
			char* clientFile = getFileFromLine(currentClientLine);
			
			printf("clientFile: %s\n", clientFile);
			printf("serverFile: %s\n", serverFile);


			//if project found on client side
			if(strcmp(clientFile,serverFile) == 0)
			{
				printf("proj found\n");

				//free(currentClientLine);
				//free(clientFile);
				close(cliManifestFd);
				break;
			}
		} 
	}
	close(servManifestFd);
	close(updateFd);
	
	remove(".servManifest");
	return 0;
}

int upgrade(char* project) //***
{
	//Checks that .Update exists
	int updateFd = open(".Update", O_RDONLY);
	if(updateFd < 0)
	{
		printf("Error: .Update does not exist.");
		return -1;
	}
	close(updateFd);
	
	//Checks that .Conflict doesn't exist
	int conflictFd = open(".Conflict", O_RDONLY);
	if(conflictFd >= 0)
	{
		printf("Error: .conflict exists.");
		close(conflictFd);
		return -1;
	}
	
	//Composes message to send to server
	char* command = malloc(7);
	char* delim = malloc(1);
	command = "upgrade";
	delim = ":";
	char* message = malloc(strlen(project)+7+1);
	strcpy(message, command);
	strcat(message, delim);
	strcat(message, project);
	
	//Connects to server, sends message
	int sockfd = connectToServer();
	if(sockfd < 0)
	{
		printf("Error: could not connect to server.\n");
		return -1;
	}
	int newsockfd = sendToServer(sockfd, message);
	
	//Reads response
	int status = 1, readIn = 0;
	char response[50];
	do{
	 status = read(newsockfd, response+readIn, 50 - readIn);
	 readIn += status;
 	} while(status > 0 && readIn < 50);

	//On success, read .Update line by line
	if(strcmp(response, "success") == 0)
	{
		int fd = open(".Update", O_RDONLY);
		file* fileLL = NULL; 
		
		//Read each line of .Update
		while(1)
		{
			char* currentLine = readUntilDelim(fd, '\n');
			
			//End of .Update reached
			if(currentLine == NULL || strlen(currentLine) == 0)
			{
				break;
			}
			
			//Case: must modify code
			if(currentLine[0] == 'M')
			{
				//Add to LL to send to server
				char* name = getFileFromLine(currentLine);
				fileLL = addFileToLL(fileLL, name);
			}
			
			//Case: must add code
			else if(currentLine[0] == 'A')
			{
				//Add to LL to send to server
				char* name = getFileFromLine(currentLine);
				fileLL = addFileToLL(fileLL, name);
			}
			
			//Case: must delete code
			else if(currentLine[0] == 'D')
			{
				//Extracts name/path from line
				char* name = getFileFromLine(currentLine);
				
				//Removes file from local project
				remove(name);
				
				//Must find location of project entry in .Manifest
				
				//Adds spaces to beginning and end of name
					//Accounts for cases where path shows up in within another path
				char* pathToFind = malloc(strlen(name)+2);
				char* space = malloc(1);
				space = " ";
				strcpy(pathToFind, space);
				strcat(pathToFind, name);
				strcpy(pathToFind, space);
				
				//Finds location of path in .Manifest string
				char* manifestString = readFromFile(".Manifest");
				char* nameInManifest = strstr(manifestString, pathToFind);
				int position = nameInManifest - manifestString;
				
				//Must split .Manifest string into beforeEntry and afterEntry
				
				//Finds first and last indices of beforeEntry chunk
				int beforeEntryFirst = 0; //Starts at beginning
				int beforeEntryLast = position - 3; //-3 skips version and new line
				
				//Finds first and last indices of afterEntry chunk
				int i = position;
				while(i < strlen(nameInManifest) && nameInManifest[i] != '\n')
				{
					//This loop finds the first new line after the name
					i++;
				}
				
				int afterEntryFirst = i; //Starts at first new line after the name
				int afterEntryLast = strlen(manifestString)-1; //Ends at final character
				int afterEntrySize = afterEntryLast - afterEntryFirst - 1;
				
				//Create strings for first chunk and second chunk
				char* newManifestBefore = malloc(beforeEntryLast);
				char* newManifestAfter = malloc(afterEntrySize);
				
				//Sets new strings char by char
				int j = 0, k = 0;
				for(j = 0; j < beforeEntryLast; j++)
				{
					newManifestBefore[j] = manifestString[j];
				}
				for(k = 0; k < afterEntrySize; k++)
				{
					newManifestAfter[k] = manifestString[k+afterEntryFirst];
				}
				
				//Replace .Manifest and write data
				remove(".Manifest");
				int manifestFd = open(".Manifest", O_RDWR|O_CREAT|O_APPEND, 0777);
				writeLoop(manifestFd, newManifestBefore, beforeEntryLast);
				writeLoop(manifestFd, newManifestAfter, afterEntrySize);
			}
		}
		
		int sockFd = connectToServer();
		if(sockfd < 0)
		{
			printf("Error: could not connect to server.\n");
			return -1;
		}
		transferOver(sockfd, fileLL, command);

		//Extracts information about upcoming data in from socket
		char* command = readUntilDelim(newsockfd, ':');
		char* numFilesString = readUntilDelim(newsockfd, ':');
		int numFiles = atoi(numFilesString);
		
		//Loops through file data coming from socket
		int i = 0, status = 1, readIn = 0;
		for(i = 0; i < numFiles; i++)
		{
			//Reads in file name
			char* nameLenString = readUntilDelim(newsockfd, ':');
			int nameLen = atoi(nameLenString);
			char* fileName = malloc(nameLen);
		  do
			{
		    status = read(newsockfd, fileName+readIn, nameLen - readIn);
		    readIn += status;
		  } while(status > 0 && readIn < nameLen);
			
			//Resets status and readIn
			status = 1;
		  readIn = 0;
			
			//Reads in file data
		  char* fileLenString = readUntilDelim(newsockfd, ':');
			int fileLen = atoi(fileLenString);
		  char* fileData = malloc(fileLen);
			do
			{
		    status = read(newsockfd, fileData+readIn, fileLen - readIn);
		    readIn += status;
		  } while(status > 0 && readIn < fileLen);
			
			//Resets status and readIn
			status = 1;
		  readIn = 0;
			
			//Opens file and writes data
			int fd = open(fileName, O_RDONLY);
			
			if(fd >= 0)
			{
				//File exists; must delete to allow for rewriting
				remove(fileName);
			}
			
			fd = open(fileName, O_RDWR|O_CREAT|O_APPEND, 0777);
			writeLoop(fd, fileData, fileLen);
		}		
	}
	else
	{
		printf("Error: Project not found on server.");
		return -1;
	}
	
	return 0;
}

int commit(char* project) //***
{
	char* updatePath = malloc(strlen(project)+10);
	char* slash = (char*)malloc(1);
	char* dot = (char*)malloc(1);
	char* updateFile = (char*)malloc(7);
	updateFile = ".Update";
	slash = "/";
	dot = ".";
	
	strcpy(updatePath, dot);
	strcat(updatePath, slash);
	strcat(updatePath, project);
	strcat(updatePath, slash);
	strcat(updatePath, updateFile);
	
	char* updateContents = readFromFile(updatePath);
	if(strcmp(updateContents, "EMPTY_FILE") != 0)
	{
		printf("Error: .Update exists and is not empty.");
		return -1;	
	}
	
	char* conflictPath = malloc(strlen(project)+10);
	char* conflictFile = (char*)malloc(10);
	conflictFile = ".Conflict";
	
	strcpy(conflictPath, dot);
	strcat(conflictPath, slash);
	strcat(conflictPath, project);
	strcat(conflictPath, slash);
	strcat(conflictPath, conflictFile);
	
	int conflictFd = open(conflictPath, O_RDONLY);
	if(conflictFd >= 0)
	{
		close(conflictFd);
		printf("Error: .Conflict exists.");
		return -1;
	}
	close(conflictFd);
	
	//Composes message for server
	char* clientcommand = malloc(6);
	char* delim = malloc(1);
	clientcommand = "update";
	delim = ":";
	char* message = malloc(strlen(project)+6+1);
	strcpy(message, clientcommand);
	strcat(message, delim);
	strcat(message, project);
	
	//Connects to server, sends message
	int sockfd = connectToServer();
	if(sockfd < 0)
	{
		printf("Error: could not connect to server.\n");
		return -1;
	}
	
	//Reads server response 
	//Extracts information about upcoming data in from socket
	char* numBytesString = readUntilDelim(sockfd, ':');
	if(strcmp(numBytesString, "error") == 0)
	{
		printf("Error: project does not exist.");
		return -1;
	}
	
	//On success, get number of bytes to read
	int numBytes = atoi(numBytesString);
	
	//Reads in file data
	char* manifestData = (char*)malloc(numBytes);

	int status = 1;
	int readIn = 0;
		
	do
	{
		  status = read(sockfd, manifestData+readIn, numBytes - readIn);
		  readIn += status;
	} while(status > 0 && readIn < numBytes);
	
	char* clientManifest = (char*)malloc(strlen(project)+12);
	char* manifest = (char*)malloc(9);
	manifest = ".Manifest";
	
	strcpy(clientManifest, dot);
	strcat(clientManifest, slash);
	strcat(clientManifest, project);
	strcat(clientManifest, slash);
	strcat(clientManifest, manifest);
	
	char* serverManifest = malloc(13);
	serverManifest = ".servManifest";
	
	int fd = open(serverManifest, O_RDWR|O_CREAT|O_APPEND, 0777);
	write(fd, manifestData, strlen(manifestData));
	close(fd);
	
	//Opens both client and server .Manifest files
	int cliManifestFd = open(clientManifest, O_RDONLY);
	int servManifestFd = open(serverManifest, O_RDONLY); 
	
	//Extracts version numbers
	char* cManifestVer = readUntilDelim(cliManifestFd, '\n');
	char* sManifestVer = readUntilDelim(servManifestFd, '\n');
	
	//Compares version numbers
	if(strcmp(cManifestVer, sManifestVer) != 0)
	{
		//Case 1: client and server manifests do not match
		printf("Error: Please update local project.");
		return -1;
	}
	
	free(cManifestVer);
	free(sManifestVer);
	close(servManifestFd);
	
	char* commitPath = (char*)malloc(strlen(project)+12);
	char* commitFile = (char*)malloc(9);
	commitFile = ".Commit";
	
	strcpy(commitPath, dot);
	strcat(commitPath, slash);
	strcat(commitPath, project);
	strcat(commitPath, slash);
	strcat(commitPath, commitFile);
	int commitFd = open(commitPath, O_RDWR|O_CREAT|O_APPEND, 0777);
	
	//Case 2: client and server manifests match
	while(1)
	{
		printf("checkpoint: manifest vers match\n");
		//Extracts line from client .Manifest and gets the project path
		char* currentClientLine = readUntilDelim(cliManifestFd, '\n');
		printf("checkpoint1\n");

		//Closes fd and exits loop if no more lines
		if(currentClientLine == NULL || strlen(currentClientLine) == 0)
		{
			//free(currentClientLine);
			close(cliManifestFd);
			break;
		}
		
		//Extracts details from line
		char* clientFile = getFileFromLine(currentClientLine);
		char* storedCliHash = getHashFromLine(currentClientLine);
		//printf("clientFile: %s\n", clientFile);
		char* filePath = malloc(strlen(project)+strlen(clientFile)+1);// proj1/example1.txt

		strcpy(filePath, project);
		strcat(filePath, slash);
		strcat(filePath, clientFile);
						
		printf("filePath: %s\n", filePath);
						
		char* currentFileString = readFromFile(filePath);
	 	char* liveHash = hash(currentFileString);

		
		//Opens server Manifest and skips over version number
		servManifestFd = open(serverManifest, O_RDONLY);
		readUntilDelim(servManifestFd, '\n');
		
		//Loops through each line of server Manifest to check if project exists
		while(1)
		{
			printf("checkpoint: inner loop\n");

			//Extracts line from server .Manifest
			char* currentServerLine = readUntilDelim(servManifestFd, '\n');
			char* servHash = getHashFromLine(currentServerLine); //append with server hash???
			
			//Case 2.1: client project does not exist on server side -- must add code
			if(currentServerLine == NULL || strlen(currentServerLine) == 0)
			{
				printf("checkpoint: client project does not exist on server side -- must add code\n");

				char action[256]; //malloc(strlen(clientFile)+liveHashSize+sizeof(char)*4); //+3 to account spaces and actionCode
				char* actionCode = malloc(sizeof(char)*2);
				char* space = malloc(sizeof(char)*1);
				char* newLine = malloc(sizeof(char)*1);
				actionCode = "A ";
				space = " ";
				newLine = "\n";
								
				strcpy(action, actionCode);
				strcat(action, clientFile);
				printf("%s\n", action);
				strcat(action, space);
				strcat(action, storedCliHash);
				strcat(action, newLine);
				writeLoop(commitFd, action, strlen(action));
				printf("written action: %s\n", action);
				
			//	memset(action, '\0', strlen(action));
				//memset(currentClientLine, '\0', strlen(currentClientLine));
				//memset(serverFile, '\0', strlen(serverFile));
				//memset(servHash, '\0', strlen(servHash));
				
				//free(action);
				//free(currentClientLine);
				////free(serverFile);
				////free(servHash);
				
				close(servManifestFd);
				break;
			}
			
			//Extracts details from line
			char* serverFile = getFileFromLine(currentServerLine);
			
			//Case 2.2: project exists on client and server
			if(strcmp(clientFile,serverFile) == 0)
			{
				printf("checkpoint:proj exists on both sides\n");

				//Case 2.2.1: server hash matches stored hash
				if(strcmp(servHash, storedCliHash) == 0)
				{
					printf("live hash: %s\n", liveHash);
					//Case 2.2.1 extended: live hash does not match stored hash -- must modify code
					if(strcmp(liveHash, storedCliHash) != 0)
					{
						char action[256]; //malloc(strlen(clientFile)+liveHashSize+sizeof(char)*4); //+3 to account spaces and actionCode
						char* actionCode = malloc(sizeof(char)*2);
						char* space = malloc(sizeof(char)*1);
						char* newLine = malloc(sizeof(char)*1);
						actionCode = "M ";
						space = " ";
						newLine = "\n";
						
						printf("serv hash: %s\n", servHash);
						
						strcpy(action, actionCode);
						strcat(action, clientFile);
						printf("%s\n", action);
	  				strcat(action, space);
						strcat(action, servHash);
						strcat(action, newLine);
						writeLoop(commitFd, action, strlen(action));
						printf("written action: %s\n", action);
						////free(actioncode);
						//free(currentServerLine);
						//free(servHash);
						//free(serverFile);
						//free(currentClientLine);
						//free(clientFile);
						//free(storedCliHash);
						//free(liveHash);
						close(servManifestFd);
						break;
					}
				}
				
				//Case 2.2.2: server hash does not match stored hash
				else
				{
					int clientFileVer = atoi(getVersionFromLine(currentClientLine));
					int serverFileVer = atoi(getVersionFromLine(currentServerLine));
					
					//Case 2.2.2 extended: server file version is greater than client file version
					if(serverFileVer > clientFileVer)
					{
						printf("Error: Failed to commit. Please synch client with repository before committing changes.");
						remove(commitPath);
						//free(currentClientLine);
						//free(currentServerLine);
						//free(servHash);
						//free(serverFile);
						//free(clientFile);
						//free(storedCliHash);
						//free(liveHash);
						close(servManifestFd);
						close(commitFd);
						return -1;
					}
				}
			}
			// free(currentServerLine);
			// free(servHash);
			// free(serverFile);
		}
	}
	close(cliManifestFd);
	
	//Open server manifest again
	servManifestFd = open(serverManifest, O_RDONLY);
	readUntilDelim(servManifestFd, '\n');
	
	//Loop through server .Manifest
	while(1)
	{
		printf("checkpoint: looping thru serv\n");

		char* currentServerLine = readUntilDelim(servManifestFd, '\n');
		
		//Closes fd and exits loop if no more lines
		if(currentServerLine == NULL || strlen(currentServerLine) == 0)
		{
			printf("done loopin\n");
			//free(currentServerLine);
			close(servManifestFd);
			break;
		}
		
		char* serverFile = getFileFromLine(currentServerLine);
		char* servHash = getHashFromLine(currentServerLine);
		
		//Opens client Manifest and skips over version number
		cliManifestFd = open(clientManifest, O_RDONLY);
		readUntilDelim(cliManifestFd, '\n');
		
		while(1)
		{
			printf("checkpoint: looping thru cli\n");
			char* currentClientLine = readUntilDelim(cliManifestFd, '\n');
			char* clientFile = getFileFromLine(currentClientLine);
			
			//Case 2.3: server has files not on client side -- must delete code
			if(currentClientLine == NULL || strlen(currentClientLine) == 0)
			{
				printf("checkpoint: must delete code\n");
				//char* storedCliHash = getHashFromLine(currentClientLine);
				char action[256]; //+4 to account spaces and actionCode
				char* actionCode = malloc(sizeof(char)*2);
				char* space = malloc(sizeof(char)*1);
				char* newLine = malloc(sizeof(char)*1);
				actionCode = "D ";
				space = " ";
				newLine = "\n";
				
				strcpy(action, actionCode);
				strcat(action, serverFile);
				
				printf("%s\n", action);
				
				strcat(action, space);
				strcat(action, servHash);
				strcat(action, newLine);
				
				writeLoop(commitFd, action, strlen(action));
				printf("written action: %s\n", action);
				
				close(cliManifestFd);
				break;
			}
		
			//if project found on client side
			if(strcmp(clientFile,serverFile) == 0)
			{
				printf("checkpoint: proj found\n");
				// free(currentServerLine);
				// free(serverFile);
				// free(servHash);
				// free(currentClientLine);
				// free(clientFile);
				close(cliManifestFd);
				break;
			}
		} 
	}
	
	close(servManifestFd);
	close(commitFd);

	return 0;
}

int push(char* project)
{
	//Checks that .Commit exists
	char* commitPath = malloc(strlen(project)+10);
	char* slash = (char*)malloc(1);
	char* dot = (char*)malloc(1);
	char* commitFile = (char*)malloc(7);
	commitFile = ".Commit";
	slash = "/";
	dot = ".";
	
	strcpy(commitPath, dot);
	strcat(commitPath, slash);
	strcat(commitPath, project);
	strcat(commitPath, slash);
	strcat(commitPath, commitFile);
	int commitFd = open(commitPath, O_RDONLY);
	if(commitFd < 0)
	{
		printf("Error: .Commit does not exist.");
		close(commitFd);
		return -1;
	}
	close(commitFd);
	
	char* commitString = readFromFile(commitPath);
	char* commitHash = hash(commitString);
	
	//Composes message to send to server
	char* message = malloc(strlen(project)+strlen(commitHash)+4+1);
	char* command = malloc(4);
	char* delim = malloc(1);
	command = "push";
	delim = ":";
	
	strcpy(message, command);
	strcat(message, delim);
	strcat(message, project);
	strcat(message, delim);
	strcat(message, commitHash);
	
	//Connects and sends to server
	int sockfd = connectToServer();
	if(sockfd < 0)
	{
		printf("Error: could not connect to server.\n");
		return -1;
	}
	int newsockfd = sendToServer(sockfd, message);
	
	//Reads server response (which is a request for files)
	char* numFilesString = readUntilDelim(newsockfd,':');
	if(strcmp(numFilesString, "error") == 0)
	{
		printf("Error: failed to commit.\n");
	}
	
	file* fileLL = NULL;
	int numFiles = atoi(numFilesString);
	int i = 0, status = 1, readIn = 0;
	for(i = 0; i < numFiles; i++)
	{
		//Reads in file name
		char* nameLenString = readUntilDelim(newsockfd, ':');
		int nameLen = atoi(nameLenString);
		char* fileName = malloc(nameLen);
	  do
		{
	    status = read(newsockfd, fileName+readIn, nameLen - readIn);
	    readIn += status;
	  } while(status > 0 && readIn < nameLen);
		
		//Resets status and readIn
		status = 1;
	  readIn = 0;
		
		fileLL = addFileToLL(fileLL, fileName);
	}
	
	//Connect to server and send over files
	sockfd = connectToServer();
	transferOver(sockfd, fileLL, command);
	
	status = 1;
	readIn = 0;
	char response[50];
	do{
	 status = read(sockfd, response+readIn, 50 - readIn);
	 readIn += status;
 } while(status > 0 && readIn < 50);
	
	if(strcmp(response, "success") != 0)
	{
		printf("Error: failed to commit.\n");
	}
	
	remove(".Commit");
	
	return 0;
}

int create(char* project) //Tested
{
	//Composes message to send to server
	char* command = malloc(6);
	char* delim = malloc(1);
	command = "create";
	delim = ":";
	char* message = malloc(strlen(project)+6+1);
	strcpy(message, command);
	strcat(message, delim);
	strcat(message, project);
	
	//Connect and send to server
	int sockfd = connectToServer();
	if(sockfd < 0)
	{
		printf("Error: could not connect to server.\n");
		return -1;
	}
	int newsockfd = sendToServer(sockfd, message);
	
	//Reads response from server
	int status = 1, readIn = 0;
	char response[50];
	do{
     status = read(newsockfd, response+readIn, 50 - readIn);
     readIn += status;
	 } while(status > 0 && readIn < 50);
	 
	 close(sockfd);
	 close(newsockfd);
	
	//On success, creates directory
	if(strcmp(response, "success") == 0)
	{
		int mkdirStatus = mkdir(project, 0777); 
		if(mkdirStatus < 0)
		{
			printf("Error: failed to create project.\n");
			return -1;
		}
		
		//Creates .Manifest in project, sets version to 0
		char* manifestPath = (char*)malloc(strlen(project)+12);
		char* slash = (char*)malloc(1);
		char* dot = (char*)malloc(1);
		char* manifest = (char*)malloc(9);
		manifest = ".Manifest";
		slash = "/";
		dot = ".";
		strcpy(manifestPath, dot);
		strcat(manifestPath, slash);
		strcat(manifestPath, project);
		strcat(manifestPath, slash);
		strcat(manifestPath, manifest);
		
		int fd = open(manifestPath, O_RDWR|O_CREAT|O_APPEND, 0777);
		
		char* version = malloc(2);
		version = "0\n";
		write(fd, version, 2);
		
		close(fd);
		close(mkdirStatus);
	}
	else
	{
		printf("Error: Failed to create project.\n");
		return -1;
	}
	return 0;
}

int destroy(char* project) //Depends on server
{
	//Composes message for server
	char* command = malloc(7);
	char* delim = malloc(1);
	command = "destroy";
	delim = ":";
	char* message = malloc(strlen(project)+7+1);
	strcpy(message, command);
	strcat(message, delim);
	strcat(message, project);
	
	//Connects to server, sends message
	int sockfd = connectToServer();
	if(sockfd < 0)
	{
		printf("Error: could not connect to server.\n");
		return -1;
	}
	int newsockfd = sendToServer(sockfd, message);
	
	//Reads response
	int status = 1, readIn = 0;
	
	char response[50];
	do{
     status = read(newsockfd, response+readIn, 50 - readIn);
     readIn += status;
	 } while(status > 0 && readIn < 50);
	
	close(sockfd);
	close(newsockfd);
	
	//On success, do nothing
	if(strcmp(response, "success") == 0)
	{
		return 0;
	}
	//On failure, print error
	else
	{
		printf("Error: Could not destroy.\n");
	}	
	
	return 0;
}

int add(char* project, char* file) //Tested
{
	//Builds file path from given info
	char* path = malloc(strlen(project)+strlen(file)+2);
	char* slash = malloc(1);
	char* dot = malloc(1);
	slash = "/";
	dot = ".";
	
	strcpy(path, dot);
	strcat(path, slash);
	strcat(path, project);
	strcat(path, slash);
	strcat(path, file);
	
	//Opens file
  int status = open(path, O_RDONLY);
	if(status < 0)
	{
		printf("Error: file does not exist on the client.\n");
		return -1;
	}
	close(status);
	
	//Opens project .Manifest
	char* manifestPath = malloc(strlen(project)+12);
	char* manifest = malloc(9);
	manifest = ".Manifest";
	strcpy(manifestPath, dot);
	strcat(manifestPath, slash);
	strcat(manifestPath, project);
	strcat(manifestPath, slash);
	strcat(manifestPath, manifest);
	
	int fd = open(manifestPath, O_RDWR|O_CREAT|O_APPEND, 0777);
	readUntilDelim(fd, '\n'); //Skips over version number
	
	while(1)
	{
		char* currentLine = readUntilDelim(fd, '\n');
		
		//End of .Manifest reached without having found project
		if(currentLine == NULL || strlen(currentLine) == 0)
		{
			char* toWrite = malloc(2+strlen(path)+1+40+1);
			char* version = malloc(1);
			char* newLine = malloc(1);
			char* space = malloc(1);
			version = "0";
			newLine = "\n";
			space = " ";
			
			char* projectString = readFromFile(path);
			char* projHash = hash(projectString);
			
			strcpy(toWrite, version);
			strcat(toWrite, space);
			strcat(toWrite, path);
			strcat(toWrite, space);
			strcat(toWrite, projHash);
			strcat(toWrite, newLine);
			
			write(fd, toWrite, strlen(toWrite));
			
			break;
		}
		
		char* fileName = getFileFromLine(currentLine);
			
		//If file is already in .Manifest, update?
		if(strcmp(fileName,path) == 0)
		{
			printf("File already exists in .Manifest.\n");
			return -1;
		}
		
	}
	close(fd);
	return 0;
}

int remove_(char* project, char* file) //Tested
{
	//Builds file path from given info
	char* path = malloc(strlen(project)+strlen(file)+2);
	char* slash = malloc(1);
	char* dot = malloc(1);
	slash = "/";
	dot = ".";
	
	strcpy(path, dot);
	strcat(path, slash);
	strcat(path, project);
	strcat(path, slash);
	strcat(path, file);
	
	int status; 
  status = open(path, O_RDONLY);
	if(status < 0)
	{
		printf("Error: file does not exist.");
		return -1;
	}
	else
	{
		//Opens project .Manifest
		char* manifestPath = malloc(strlen(project)+12);
		char* manifest = malloc(9);
		manifest = ".Manifest";
		strcpy(manifestPath, dot);
		strcat(manifestPath, slash);
		strcat(manifestPath, project);
		strcat(manifestPath, slash);
		strcat(manifestPath, manifest);
		
		//Must find location of project entry in .Manifest
				
		//Adds spaces to beginning and end of name
			//Accounts for cases where path shows up in within another path
		char* pathToFind = malloc(strlen(path)+2);
		char* space = malloc(1);
		space = " ";
		strcpy(pathToFind, space);
		strcat(pathToFind, path);
		strcpy(pathToFind, space);
		
		//Finds location of path in .Manifest string
		char* manifestString = readFromFile(manifestPath);
		char* nameInManifest = strstr(manifestString, pathToFind);
		int position = nameInManifest - manifestString;
		
		//Must split .Manifest string into beforeEntry and afterEntry
				
		//Finds first and last indices of beforeEntry chunk
		int beforeEntryFirst = 0; //Starts at beginning
		int beforeEntryLast = position-1; //-3 skips version and new line
				
		//Finds first and last indices of afterEntry chunk
		int i = position;
		while(i < strlen(nameInManifest) && nameInManifest[i] != '\n')
		{
			//This loop finds the first new line after the name
			i++;
		}
				
		int afterEntryFirst = i+4; //Starts at first new line after the name
		int afterEntryLast = strlen(manifestString)-1; //Ends at final character
		int afterEntrySize = afterEntryLast - afterEntryFirst - 1;
				
		//Create strings for first chunk and second chunk
		char* newManifestBefore = malloc(beforeEntryLast);
		char* newManifestAfter = malloc(afterEntrySize);
		memset(newManifestBefore, '\0', beforeEntryLast);
		memset(newManifestAfter, '\0', afterEntrySize);
				
		//Sets new strings char by char
		int j = 0, k = 0;
		for(j = 0; j < beforeEntryLast; j++)
		{
			newManifestBefore[j] = manifestString[j];
			//printf("newManifestBefore[j] = %c\n",newManifestBefore[j]);
		}
		for(k = 0; k < afterEntrySize; k++)
		{
			newManifestAfter[k] = manifestString[k+afterEntryFirst];
		}
		
		//printf("before: %s\nafter: %s\n", newManifestBefore, newManifestAfter);
			
		//Replace .Manifest and write data
		remove(manifestPath);
		int manifestFd = open(manifestPath, O_RDWR|O_CREAT|O_APPEND, 0777);
		writeLoop(manifestFd, newManifestBefore, beforeEntryLast);
		writeLoop(manifestFd, newManifestAfter, afterEntrySize);

	}
	
	return 0;
}

int currentversion(char* project) //Depends on server
{
	//Composes message for server
	char* clientcommand = malloc(14);
	char* delim = malloc(1);
	clientcommand = "currentversion";
	delim = ":";
	char* message = malloc(strlen(project)+6+1);
	strcpy(message, clientcommand);
	strcat(message, delim);
	strcat(message, project);
	
	//Connects to server, sends message
	int sockfd = connectToServer();
	if(sockfd < 0)
	{
		printf("Error: could not connect to server.\n");
		return -1;
	}
	
	//Reads server response 
	
	//Extracts information about upcoming data in from socket
	char* numBytesString = readUntilDelim(sockfd, ':');
	
	if(strcmp(numBytesString, "error") == 0)
	{
		char* cause = readUntilDelim(sockfd, ' ');
		if(cause[0] == 'e')
		{
			printf("Error: project is empty.");
		}
		else
		{
			printf("Error: project does not exist.");
		}
		
		return -1;
	}
	
	//On success, get number of bytes to read
	int numBytes = atoi(numBytesString);
	
	//Reads in file data
	char* data = (char*)malloc(numBytes);

	int status = 1;
	int readIn = 0;
		
	do
	{
		  status = read(sockfd, data+readIn, numBytes - readIn);
		  readIn += status;
	} while(status > 0 && readIn < numBytes);
	
	printf("%s\n", data);
	
	return 0;
}

int history(char* project) //Depends on server
{
	//Composes message to send to server
	char* command = malloc(7);
	char* delim = malloc(1);
	command = "history";
	delim = ":";
	char* message = malloc(strlen(project)+7+1);
	strcpy(message, command);
	strcat(message, delim);
	strcat(message, project);
	
	//Connects to server, sends message
	int sockfd = connectToServer();
	if(sockfd < 0)
	{
		printf("Error: could not connect to server.\n");
		return -1;
	}
	int newsockfd = sendToServer(sockfd, message);
	int status = 1, readIn = 0;

	char* numBytesString = readUntilDelim(newsockfd, ':');
	
	if(strcmp(numBytesString, "error") == 0)
	{
		printf("Error: Project does not exist on server.");
		return -1;
	}
	
	//Reads response
	int numBytes = atoi(numBytesString);
	char* output = malloc(numBytes);
  status = 1, readIn = 0;
	do{
	 status = read(newsockfd, output+readIn, numBytes - readIn);
	 readIn += status;
 	} while(status > 0 && readIn < numBytes);
	
	printf("%s\n", output);
	
	return 0;
}

int rollback(char* project, char* version) //Should work, depends on server
{
	//Composes message
	char* command = malloc(7);
	char* delim = malloc(1);
	command = "rollback";
	delim = ":";
	char* message = malloc(strlen(project)+strlen(version)+7+1);
	strcpy(message, command);
	strcat(message, delim);
	strcat(message, project);
	strcat(message, version);
	
	//Connects to server, sends message
	int sockfd = connectToServer();
	if(sockfd < 0)
	{
		printf("Error: could not connect to server.\n");
		return -1;
	}
	int newsockfd = sendToServer(sockfd, message);
	
	//Reads response
	char* response = readUntilDelim(newsockfd, ':');
	if(strcmp(response, "error"))
	{
		char* errorCause = readUntilDelim(newsockfd, '\n');
		if(errorCause[0] == 'i')
		{
			printf("Error: Invalid version number.");
			return -1;
		}
		else
		{
			printf("Error: Project does not exist.");
			return -1;
		}
	}
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

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
	if(sockfd < 0)
	{
		return -1;
	}
	struct hostent* host = gethostbyname("127.0.0.1");
	struct sockaddr_in serverAddressInfo;
	
	bzero((char*)&serverAddressInfo, sizeof(serverAddressInfo));
	
	serverAddressInfo.sin_family = AF_INET;
	serverAddressInfo.sin_addr.s_addr = htons(INADDR_ANY);
	serverAddressInfo.sin_port = htons(atoi(PORT));
	
	bcopy((char*)host->h_addr, (char*)&serverAddressInfo.sin_addr.s_addr, host->h_length);
	
	cxn_status = connect(sockfd, (struct sockaddr *)&serverAddressInfo, sizeof(serverAddressInfo));
	if(cxn_status < 0)
	{
		return -1;
	}
	
	return sockfd;
}

//Composes a message based on delimiter-based protocol
/*char* composeMessage(char* command, file* arr, char* numFiles)
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
	 	sizeOfBuffer += strlen(curr->fileLen); //len of numBytes
	 	sizeOfBuffer += atoi(curr->fileLen); //numBytes
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
	 	strcat(buffer, curr->fileLen);
	 	strcat(buffer, delim);
		strcat(buffer, curr->fileData);
  }
	
	printf("buffer: %s\n", buffer);
  return buffer;
	
}*/

int sendToServer(int sockfd, char* message)
{
	char* buffer[256]; //check type
	int n = 0;
	bzero(buffer,256);
	
	n = write(sockfd,message,strlen(message));
	if (n < 0) 
		 printf("ERROR writing to socket\n");
	
	//printf("Message: %s\n", buffer);
	// RETURN SOCK FD
	return 0;
}

// Reads entire file into string buffer
// Returns NULL if file does not exist, string otherwise
char* readFromFile(char* file)
{
    int fd = open(file, O_RDONLY);    // Returns -1 on failure, >0 on success
    // Fatal Error if file does not exist
    if(fd < 0){
        //printf("Fatal Error: File does not exist.\n");
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
        //printf("Warning: Empty file.\n");
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

    //free(buffer);
    return fileBuffer;
}

// Hashes a given string and returns the code 
char* hash(char* data)
{
	int x = 0;
	size_t length = strlen(data);
	char* buffer = malloc(40);
	memset(buffer, '\0', 40);
	unsigned char hash[SHA_DIGEST_LENGTH];
	//SHA1(data, length, hash);
	
	for(x = 0; x < 30; x++)
	{
		sprintf(buffer+(x * 2), "%02x", hash[x]);
	}
	
	//not sure why this is needed but it is:
	char* finalbuffer = malloc(40);
	memset(finalbuffer, '\0', 40);
	
	for(x = 0; x < 40; x++)
	{
		finalbuffer[x] = buffer[x];
	}
	buffer[40] = '\0';
	
	printf("hash code: %s\n", buffer);
	//free(buffer);
	
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
	
	char* buffer = malloc(41);
	//char clientbuffer[end-start+1];
	memset(buffer, '\0', strlen(buffer));
	
	for(i = 0; i < 40; i++)
	{
		buffer[i] = line[i+start];
	}

	printf("getHashFromLine buffer: %s\n", buffer);
	return buffer;
}

// Extracts project path from .Manifest line
// Returns path string
char* getFileFromLine(char* line)
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
			addDirToLL(fileLL, buff);
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