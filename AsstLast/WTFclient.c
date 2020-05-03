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

char* hash(char* data);

char* getHashFromLine(char* line);
char* getFileFromLine(char* line);
char* getVersionFromLine(char* line);

char* readFromFile(char* file);
char* readUntilDelim(int fd, char delim);
void writeLoop(int fd, char* str, int numBytes);
char* intToString(int num);

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
	command = "proj1";
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
	
	//sendToServer(sockfd, command);
	//composeMessage(command, arr, numFiles);
	//getHash(command);
	//commit(command);
	checkout(command);
	
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
	
	writeLoop(fd, buffer, strlen(buffer));

	return 0;
}

int checkout(char* project)
{
	/*The checkout command will fail if the project name doesn’t exist on the server, 
	the client can't communicate with the server,
	if the project name already exists on the client side 
	if configure was not run on the client side.*/
	
	int sockfd = connectToServer();
	char message[20] = "checkout:project";
	int newsockfd = sendToServer(sockfd, message);
	int status = 1, readIn = 0;
	
	char* command = readUntilDelim(newsockfd, ':');
	char* numFilesString = readUntilDelim(newsockfd, ':');
	int numFiles = intToString(numFilesString);
	
	int i = 0;
	for(i = 0; i < numFiles; i++)
	{
		char* nameLenString = readUntilDelim(newsockfd, ':');
		int nameLen = intToString(nameLenString);
		
		char* fileName = malloc(nameLen);
    do{
        status = read(newsockfd, fileName+readIn, nameLen - readIn);
        readIn += status;
    } while(status > 0 && readIn < nameLen);
		status = 1;
    readIn = 0;
		
		char* fileLenString = readUntilDelim(sockfd, ':');
		int fileLen = intToString(nameLenString);
		
		char* fileData = malloc(fileLen);
		do{
        status = read(sockfd, fileData+readIn, fileLen - readIn);
        readIn += status;
    } while(status > 0 && readIn < nameLen);
		status = 1;
    readIn = 0;
		
		int fd = open(fileName, O_RDWR|O_CREAT|O_APPEND, 00600);
		writeLoop(fd, fileData, fileLen);
	}
	
/*project .Manifest as well as all the files that are listed in it. The client will be responsible for receiving the
project, creating any subdirectories under the project and putting all files in to place as well as saving the
.Manifest. */
	return 0;
}

int update(char* project)
{

	//failure cases
	//char* command = malloc();
	//char* message = composeMessage(); //get manifest
	
	// TESTING 
	
	char* clientManifest = malloc(30);
	char* serverManifest = malloc(30);
	clientManifest = "proj1/.Manifest";
	serverManifest = "Server/proj1/.Manifest";
	
	//Opens both client and server .Manifest files
	int cliManifestFd = open(clientManifest, O_RDONLY);
	int servManifestFd = open(serverManifest, O_RDONLY); //Only used to compare versions
	
	//Extracts version numbers
	char* cManifestVer = readUntilDelim(cliManifestFd, '\n');
	char* sManifestVer = readUntilDelim(servManifestFd, '\n');
	
	//Case 1: same Manifest versions
	if(strcmp(cManifestVer, sManifestVer) == 0) 
	{
		int toBeRemoved = open(".Conflict", O_RDONLY);
		if(toBeRemoved >= 0)
		{
			remove(".Conflict");
			close(toBeRemoved);
		}
		printf("Up To Date\n");
		close(cliManifestFd);
		close(servManifestFd);
		//free(cManifestVer);
		//free(sManifestVer);
		return 0;
	}

	close(servManifestFd);
	//free(cManifestVer);
	//free(sManifestVer);
	
	int updateFd = open(".Update", O_RDWR|O_CREAT|O_APPEND, 00600);
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
							
							int conflictFd = open(".Conflict", O_RDONLY);
							if(conflictFd < 0)
							{
								conflictFd = open(".Conflict", O_RDWR|O_CREAT|O_APPEND, 00600);
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
		printf("Error: .Conflict exists.");
		return -1;
	}
	
	// TESTING 
	char* clientManifest = malloc(30);
		char* serverManifest = malloc(30);
		clientManifest = "proj1/.Manifest";
		serverManifest = "Server/proj1/.Manifest";
	
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
	
	int commitFd = open(".Commit", O_RDWR|O_CREAT|O_APPEND, 00600);
	
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
		char* slash = malloc(1);
		slash = "/";
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
						remove(".Commit");
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
	//The push command will fail if the project name doesn’t 
	//exist on the server, if the client can not communicate
	//with the server or if the client has no .Commit file
	
	send commit to servVersion
	get all files, send in linked list 	
	get response 
	erase commit
	
	return 0;
}

int create(char* project)
{
	char* command = malloc(6);
	char* delim = malloc(1);
	command = "create";
	delim = ":";
	char* message = malloc(strlen(project)+6+1);
	strcpy(message, command);
	strcat(message, delim);
	strcat(message, project);
	
	int sockfd = connectToServer();
	int newsockfd = sendToServer(sockfd, message);
	int status = 1, readIn = 0;
	
	char response[20];
	do{
     status = read(newsockfd, response+readIn, nameLen - readIn);
     readIn += status;
    } while(status > 0 && readIn < nameLen);
	
	if(strcmp(response, "success") == 0)
	{
		status = mkdir(project, 00600); 
		if(status < 0)
		{
			//failure
		}
		
		int fd = open(".Manifest", O_RDWR|O_CREAT|O_APPEND, 00600);
		char* version = malloc(1);
		version = "0";
		
		write(fd, version, 1);

	}
	else
	{
		//failure
	}
	return 0;
}

int destroy(char* project)
{
	char* command = malloc(7);
	char* delim = malloc(1);
	command = "destroy";
	delim = ":";
	char* message = malloc(strlen(project)+7+1);
	strcpy(message, command);
	strcat(message, delim);
	strcat(message, project);
	
	int sockfd = connectToServer();
	int newsockfd = sendToServer(sockfd, message);
	int status = 1, readIn = 0;
	
	char response[20];
	do{
     status = read(newsockfd, response+readIn, nameLen - readIn);
     readIn += status;
    } while(status > 0 && readIn < nameLen);
	
	if(strcmp(response, "success") == 0)
		return 0;
}

int add(char* project, char* file)
{
	int status; 
	char* path = malloc(strlen(project)+strlen(file));
	char* slash = malloc(1);
	slash = "/";
	strcpy(path,project);
	strcat(path, slash);
	strcat(path, file);
  status = open(file, O_RDONLY);
	if(status < 0)
	{
		printf("Error: file does not exist on the client.");
		return -1;
	}
	
	int fd = open(".Manifest", O_RDWR|O_CREAT|O_APPEND, 00600);
	
	find in manifest
	if it does not exist, append 
	if it does, add a code??

	// append: 0 file.txt efgjdyfasu

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
	char* command = malloc(14);
	char* delim = malloc(1);
	command = "currentversion";
	delim = ":";
	char* message = malloc(strlen(project)+6+1);
	strcpy(message, command);
	strcat(message, delim);
	strcat(message, project);
	
	int sockfd = connectToServer();
	int newsockfd = sendToServer(sockfd, message);
	int status = 1, readIn = 0;
	
	
	
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

