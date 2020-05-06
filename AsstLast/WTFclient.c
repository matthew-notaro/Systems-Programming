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
#include <math.h>

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
int currentversion(char* project);
int history(char* project);
int rollback(char* project, char* version);

int setServerDetails();
int connectToServer();
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
			printf("Configuration successful.\n");
			return 0;
		}
		return -1;
	}
	setServerDetails();
	// printf("Host: %s\n", HOST);
	// printf("Port: %s\n", PORT);

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
		if(commit(argv[2]) == 0){
			printf("Commit successful.\n");
			return 0;
		}
		return -1;
	}
	else if(strcmp(op, "push") == 0 && argc == 3){
		if(push(argv[2]) == 0){
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
	
	return 0;
}

//Saves IP address/host and port of server 
//Returns 0 on success, -1 on failure
int configure(char* IPAddress, char* portNum)
{
	int IPAddressSize = 0, portNumSize = 0, bufferSize = 0;
	char* configName = "./.Configure";
	char* space = malloc(sizeof(char)*1);
	space = " ";
	
	//Check whether .Configure already exists
	int check = open(configName, O_RDONLY);
	if(check >= 0)
	{
		remove(configName);
		printf("Re-Configuring...\n");
	}
	close(check);

	int fd = open(configName, O_RDWR|O_CREAT|O_APPEND, 0777);
	
	IPAddressSize = strlen(IPAddress) + 1;
	portNumSize = strlen(portNum) + 1;
	bufferSize = IPAddressSize+portNumSize+sizeof(char)*1;
	
	char* buffer = malloc(bufferSize);
	
	strcpy(buffer, IPAddress);
	strcat(buffer, space);
	strcat(buffer, portNum);
	
	writeLoop(fd, buffer, strlen(buffer));

	close(fd);
	return 0;
}

//Clones project from server
//Returns 0 on success, -1 on failure 
int checkout(char* project)
{
	//Checks that client has already configured
	int configureFd = open("./.Configure", O_RDONLY);
	if(configureFd < 0)
	{
		printf("ERROR: Please configure.\n");
		return -1;
	}
	
	//Checks that project does not exist on client
	DIR* projectFd = opendir(project);
	if(projectFd != NULL)
	{
		printf("ERROR: Project already exists on client.\n");
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
		printf("ERROR: Could not connect to server.\n");
		return -1;
	}
	printf("Connection to server established.\n");
	int newsockfd = sendToServer(sockfd, message);
	if(newsockfd < 0)
	{
		printf("ERROR: Could not write to socket\n");
		return -1;
	}
	
	//Reads server response 
	
	//Extracts information about upcoming data in from socket
	char* command = readUntilDelim(newsockfd, ':');
	if(strcmp(command, "error") == 0)
	{
		printf("ERROR: Project does not exist on server.\n");
		close(sockfd);
		close(newsockfd);
		return -1;
	}
	
	//Makes project directory
	int mkdirStatus = mkdir(project, 0777); 
	if(mkdirStatus < 0)
	{
		printf("ERROR: failed to create project.\n");
		return -1;
	}
	
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
		
		//Build filePath to follow format:
			//Client/<project>/<filename>
		char* filePath = malloc(strlen(fileName)+strlen(project)+3);
		char* parentDir = malloc(2);
		char* slash = malloc(1);
		parentDir = "./";
		slash = "/";
		
		strcpy(filePath, parentDir);
		strcat(filePath, project);
		strcat(filePath, slash);
		strcat(filePath, fileName);
		
		//Creates file and writes data
		int fd = open(fileName, O_RDWR|O_CREAT|O_APPEND, 0777);
		writeLoop(fd, fileData, fileLen);
		close(fd);
	}
	
	close(sockfd);
	close(newsockfd);
	return 0;
}

//Checks if server has any clients for update and edits .Manifest accordingly
//Returns 0 on success, -1 on failure
int update(char* project) 
{
	//Composes message for server
	char* clientcommand = malloc(6);
	char* delim = malloc(1);
	char* slash = (char*)malloc(1);
	char* dot = (char*)malloc(1);
	clientcommand = "update";
	delim = ":";
	slash = "/";
	dot = ".";
	
	char* message = malloc(strlen(project)+6+1);
	strcpy(message, clientcommand);
	strcat(message, delim);
	strcat(message, project);
	
	//Connects to server, sends message
	int sockfd = connectToServer();
	if(sockfd < 0)
	{
		printf("ERROR: Could not connect to server.\n");
		return -1;
	}
	printf("Connection to server established.\n");
	
	//Reads server response 
	//Extracts information about upcoming data in from socket
	char* serverResponse = readUntilDelim(sockfd, ':');
	if(strcmp(serverResponse, "error") == 0)
	{
		printf("ERROR: Project does not exist.\n");
		return -1;
	}
	
	//On success, get number of bytes to read
	char* numBytesString = readUntilDelim(sockfd, ':');
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
	
	char* clientManifest = (char*)malloc(strlen(project)+9+7);
	char* manifest = (char*)malloc(9);
	char* parentDir = malloc(7);
	manifest = ".Manifest";
	parentDir = "./";
	strcpy(clientManifest, parentDir);
	strcat(clientManifest, project);
	strcat(clientManifest, slash);
	strcat(clientManifest, manifest);
		
	char* serverManifest = malloc(13);
	serverManifest = ".servManifest";
	
	int fd = open(serverManifest, O_RDWR|O_CREAT|O_APPEND, 0777);
	writeLoop(fd, manifestData, strlen(manifestData));
	close(fd);

	//Opens both client and server .Manifest files
	int cliManifestFd = open(clientManifest, O_RDONLY);
	int servManifestFd = open(serverManifest, O_RDONLY); //Only used to compare versions

	//Extracts version numbers
	char* cManifestVer = readUntilDelim(cliManifestFd, '\n');
	char* sManifestVer = readUntilDelim(servManifestFd, '\n');
	
	//Creates .Update file
	char* updatePath = malloc(strlen(project)+7+10);
	char* updateFile = (char*)malloc(7);
	updateFile = ".Update";
	strcpy(updatePath, parentDir);
	strcat(updatePath, project);
	strcat(updatePath, slash);
	strcat(updatePath, updateFile);
		
	int updateFd = open(updatePath, O_RDWR|O_CREAT|O_APPEND, 0777);
	
	//Case 1: same Manifest versions
	if(strcmp(cManifestVer, sManifestVer) == 0) 
	{
		char* conflictPath = malloc(strlen(project)+5+12);
		char* conflictFile = (char*)malloc(9);
		conflictFile = ".Conflict";
		strcpy(conflictPath, parentDir);
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
		remove(".servManifest");
		return 0;
	}

	close(servManifestFd);

	char* currentClientLine = NULL, *clientFile = NULL, *cliVersion = NULL, *storedCliHash = NULL;

	//Case 2: different Manifest versions
	//Loops through each line of client Manifest
	while(1)
	{

		//Extracts line from client .Manifest
		currentClientLine = readUntilDelim(cliManifestFd, '\n');
		
		//Closes fd and exits loop if no more lines
		if(currentClientLine == NULL || strlen(currentClientLine) == 0)
		{
			memset(currentClientLine, '\0', strlen(currentClientLine));
			close(cliManifestFd);
			break;
		}
		
		//Extracts client details
		clientFile = getFileFromLine(currentClientLine);
		cliVersion = getVersionFromLine(currentClientLine);
		storedCliHash = getHashFromLine(currentClientLine);
	
		//Opens server Manifest and skips over version number
		servManifestFd = open(serverManifest, O_RDONLY);
		readUntilDelim(servManifestFd, '\n'); 
		
		//Loops through each line of server Manifest to check if file exists
		while(1)
		{
			//Extracts line from server .Manifest
			char* currentServerLine = NULL, *serverFile = NULL, *servVersion = NULL, *servHash = NULL;
			
			currentServerLine = readUntilDelim(servManifestFd, '\n');
			
			//Case 2.1: server has removed files from the project
			if(currentServerLine == NULL || strlen(currentServerLine) == 0)
			{
				char action[400];
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
				
				memset(action, '\0', strlen(action));
				memset(storedCliHash, '\0', strlen(storedCliHash));

				close(servManifestFd);
				break;
			}
			
			//Extracts details
			serverFile = getFileFromLine(currentServerLine);
			servVersion = getVersionFromLine(currentServerLine);
			servHash = getHashFromLine(currentServerLine);
		
			//Case 2.2: file found on server side
			if(strcmp(clientFile,serverFile) == 0)
			{

				//Checks if versions don't match
				if(strcmp(cliVersion,servVersion) != 0)
				{
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
												
						char* currentFileString = readFromFile(filePath);
						char* liveHash = hash(currentFileString);
		
				
						if(strcmp(storedCliHash,liveHash) == 0)
						{
							
							//Case 2.2.1: server has modified files
							char action[400]; 
							char* actionCode = malloc(sizeof(char)*2);
							char* space = malloc(sizeof(char)*1);
							char* newLine = malloc(sizeof(char)*1);
							actionCode = "M ";
							space = " ";
							newLine = "\n";
							
							//printf("serv hash: %s\n", servHash);
							
							strcpy(action, actionCode);
							strcat(action, clientFile);
							printf("%s\n", action);
							strcat(action, space);
							strcat(action, servHash);
							strcat(action, newLine);
							writeLoop(updateFd, action, strlen(action));
						

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
							char action[400]; 
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
							
							char* conflictPath = malloc(strlen(project)+7+10);
							char* conflictFile = (char*)malloc(10);
							conflictFile = ".Conflict";
							
							strcpy(conflictPath, parentDir);
							strcat(conflictPath, project);
							strcat(conflictPath, slash);
							strcat(conflictPath, conflictFile);	
						
							
							int conflictFd = open(conflictPath, O_RDONLY);
							if(conflictFd < 0)
							{
								conflictFd = open(conflictPath, O_RDWR|O_CREAT|O_APPEND, 0777);
						  }

							writeLoop(conflictFd, action, 400);
							
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
			char* currentClientLine = readUntilDelim(cliManifestFd, '\n');
						
			//Case 2.3: server has files not on client side
			if(currentClientLine == NULL || strlen(currentClientLine) == 0)
			{
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
				//printf("written action: %s\n", action);
				
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
			
			//printf("clientFile: %s\n", clientFile);
			//printf("serverFile: %s\n", serverFile);


			//if project found on client side
			if(strcmp(clientFile,serverFile) == 0)
			{
				//printf("proj found\n");

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

//Applies changes listed in .Update to local copy of project
//Returns 0 on success, -1 on failure
int upgrade(char* project)
{
	//Checks that .Update exists
	char* parentDir = malloc(9);
	char* slash = malloc(1);
	char* update = malloc(7);
	slash = "/";
	parentDir = "./";
	update = ".Update";
	
	char* updatePath = malloc(9+strlen(project)+7+1);
	
	strcpy(updatePath, parentDir);
	strcat(updatePath, project);
	strcat(updatePath, slash);
	strcat(updatePath, update);
	
	//printf("updatePath %s\n", updatePath);
	
	int updateFd = open(updatePath, O_RDONLY);
	if(updateFd < 0)
	{
		printf("ERROR: .Update does not exist.\n");
		return -1;
	}
	close(updateFd);
	
	//Checks that .Conflict doesn't exist
	char* conflictPath = malloc(9+7+strlen(project)+1);
	char* conflict = malloc(9);
	conflict = ".Conflict";
	
	strcpy(conflictPath, parentDir);
	strcat(conflictPath, project);
	strcat(conflictPath, slash);
	strcat(conflictPath, conflict);
	
	int conflictFd = open(conflictPath, O_RDONLY);
	if(conflictFd >= 0)
	{
		printf("ERROR: .Conflict exists.\n");
		close(conflictFd);
		return -1;
	}

	//Reads .Update line by line
	
	int fd = open(updatePath, O_RDONLY);
	file* fileLL = NULL; 
		
	//Read each line of .Update
	while(1)
	{
		char* currentLine = readUntilDelim(fd, '\n');
		//printf("checkpoint currLine = %s\n", currentLine);
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
			//printf("checkpoint M to LL\n");
		}
			
		//Case: must add code
		else if(currentLine[0] == 'A')
		{
			//Add to LL to send to server
			char* name = getFileFromLine(currentLine);
		  fileLL = addFileToLL(fileLL, name);
			//printf("checkpoint A to LL\n");
		}
			
		//Case: must delete code
		else if(currentLine[0] == 'D')
		{
				//Extracts name/path from line
				char* name = getFileFromLine(currentLine);
				//printf("checkpoint1\n");

				char* toRemove = malloc(strlen(parentDir)+strlen(project)+1);
				strcpy(toRemove, parentDir);
				strcat(toRemove, project);
				strcat(toRemove, slash);
				strcat(toRemove, name);
				
				//Removes file from local project
				remove(toRemove);
				//printf("checkpoint2\n");
				//Must find location of project entry in .Manifest
				
				//Adds spaces to beginning and end of name
					//Accounts for cases where path shows up in within another path
				char* pathToFind = malloc(strlen(name)+2);
				char* space = malloc(1);
				space = " ";
				strcpy(pathToFind, space);
				strcat(pathToFind, name);
				strcat(pathToFind, space);
				//printf("pathToFind: %s\n", pathToFind);
				//Finds location of path in .Manifest string
				
				char* manifestPath = malloc(9+7+strlen(project)+1);
				char* manifest = malloc(9);
				manifest = ".Manifest";
				
				strcpy(manifestPath, parentDir);
				strcat(manifestPath, project);
				strcat(manifestPath, slash);
				strcat(manifestPath, manifest);
				
				char* manifestString = readFromFile(manifestPath);
				char* nameInManifest = strstr(manifestString, pathToFind);
				int position = (int) (nameInManifest - manifestString);
				
				//printf("manifestString: %s\n", manifestString);
				//printf("nameInManifest: %s\n", nameInManifest);
				
				//Must split .Manifest string into beforeEntry and afterEntry
				
				//Finds first and last indices of beforeEntry chunk
				int beforeEntryFirst = 0; //Starts at beginning
				int beforeEntryLast = position - 1;
				
				//printf("checkpoint3\n");
				
				//printf("strlen(nameInManifest): %d\n", strlen(nameInManifest));
		
			 
				//Create strings for first chunk and second chunk
				char* newManifestBefore = malloc(beforeEntryLast);
				//char* newManifestAfter = malloc(afterEntrySize);
				
				char* newManifestAfter = strstr(nameInManifest, "\n");
				 
				//Sets new strings char by char
				int j = 0, k = 0;
				for(j = 0; j < beforeEntryLast-1; j++)
				{
				 	newManifestBefore[j] = manifestString[j];
				}

				//rintf("checkpoint5\n");
				// printf("newManifestBefore: %s\n", newManifestBefore);
				// printf("newManifestAfter: %s\n", newManifestAfter);
				
				//Replace .Manifest and write data
				remove(manifestPath);
				int manifestFd = open(manifestPath, O_RDWR|O_CREAT|O_APPEND, 0777);
				writeLoop(manifestFd, newManifestBefore, strlen(newManifestBefore));
				if(newManifestAfter!=NULL)
					writeLoop(manifestFd, newManifestAfter, strlen(newManifestAfter));
			}
	}

	//Sends message to server
	
	//Connects to server
	int sockfd = connectToServer();
	if(sockfd < 0)
	{
		printf("ERROR: Could not connect to server.\n");
		return -1;
	}
	printf("Connection to server established.\n");
	
	//int sockfd = 0; //comment out
	
	file* ptr = fileLL;
	int num = 0;
	
	while(1){
		if(ptr == NULL)
			break;
		// printf("loop\n");
		// printf("file %s\n", ptr->fileName);
		num++;
		ptr = ptr->next;
  }
	//printf("checkpoint\n");
	char* cmd = malloc(7);
	cmd = "upgrade";

	writeLoop(sockfd, cmd, strlen(cmd));
	write(sockfd, ":", 1);
	writeLoop(sockfd, project, strlen(project));
	write(sockfd, ":", 1);
	char* numString = intToString(num);
	writeLoop(sockfd, numString, strlen(numString));
	write(sockfd, ":", 1);
	
/*	char* numString = intToString(num);
	printf("%s:", cmd);
	printf("%s:", project);
	printf("%d:", num);*/

	ptr = fileLL;
	while(ptr != NULL){
	  char* nameLenString = intToString(ptr->nameLen);
		// printf("loop: %s:", nameLenString);
		// printf("%s", ptr->fileName);
	  writeLoop(sockfd, nameLenString, strlen(nameLenString));
		write(sockfd, ":", 1);
  	writeLoop(sockfd, ptr->fileName, ptr->nameLen);
		ptr=ptr->next;
  }
	//Extracts information about upcoming data in from socket
	char* serverResponse = readUntilDelim(sockfd, ':');
	if(strcmp(serverResponse, "error") == 0)
	{
		printf("ERROR: Project does not exist on server.\n");
		close(sockfd);
		return -1;
	}
	char* numFilesString = readUntilDelim(sockfd, ':');
	int numFiles = atoi(numFilesString);

	//Loops through file data coming from socket
	int i = 0, status = 1, readIn = 0;
	for(i = 0; i < numFiles; i++)
	{
	  //Reads in file name
		char* nameLenString = readUntilDelim(sockfd, ':');
	 	int nameLen = atoi(nameLenString);
	 	char* fileName = malloc(nameLen);
	 	do
	 	{
	 		status = read(sockfd, fileName+readIn, nameLen - readIn);
	 		readIn += status;
	 	} while(status > 0 && readIn < nameLen);
	 
	 	//Resets status and readIn
	 	status = 1;
	 	readIn = 0;
	 
	 	//Reads in file data
	 	char* fileLenString = readUntilDelim(sockfd, ':');
	 	int fileLen = atoi(fileLenString);
	 	char* fileData = malloc(fileLen);
	 	do
	 	{
	 		status = read(sockfd, fileData+readIn, fileLen - readIn);
	 		readIn += status;
	 	} while(status > 0 && readIn < fileLen);
	 
	 	//Resets status and readIn
	 	status = 1;
	 	readIn = 0;
	 
	 	char* filePath = malloc(strlen(fileName)+strlen(project)+strlen(parentDir)+1);
	 
	 	strcpy(filePath, parentDir);
	 	strcat(filePath, project);
	 	strcat(filePath, slash);
	 	strcat(filePath, fileName);
	 
	 	//Opens file and writes data
	 	int fd = open(filePath, O_RDONLY);
	 
	 	if(fd >= 0)
	 	{
	 		//File exists; must delete to allow for rewriting
	 		remove(filePath);
	 	}
	 
	 	fd = open(filePath, O_RDWR|O_CREAT|O_APPEND, 0777);
	 	writeLoop(fd, fileData, fileLen);
	 	close(fd);
	 }
	 
	char* manifestPath = malloc(9+7+strlen(project)+1);
 	char* manifest = malloc(9);
 	manifest = ".Manifest";
 	
 	strcpy(manifestPath, parentDir);
 	strcat(manifestPath, project);
 	strcat(manifestPath, slash);
 	strcat(manifestPath, manifest); 
	
	//Opens manifest
	int manifestfd = open(manifestPath, O_RDONLY);
	
	//Reads version number and increments it
	char* manifestVersionString = readUntilDelim(manifestfd, '\n');
	int newManifestVersionNum = atoi(manifestVersionString)+1;
	char* newManifestVersion = intToString(newManifestVersionNum);
	
	//Reads rest of manifest into a buffer
	char* currentManifestString = readFromFile(manifestPath);
	char* restOfManifest = malloc(strlen(currentManifestString)+4);

  int s = 1, ri = 0;
	do
	{
	  s = read(manifestfd, restOfManifest+ri, strlen(currentManifestString) - ri);
	 	ri += s;
	} while(s > 0 && ri < strlen(currentManifestString));
	
	//Replaces .Manifest with updated version
	remove(manifestPath);
	int newManifest = open(manifestPath, O_RDWR|O_CREAT|O_APPEND, 0777);
	writeLoop(newManifest, newManifestVersion, strlen(newManifestVersion));
	writeLoop(newManifest, restOfManifest, strlen(restOfManifest));
	close(manifestfd);
	return 0;
}

//Composes .Commit and sends to server to implement changes
//Returns 0 on success, -1 on failure
int commit(char* project)
{
	//Checks that .Update doesn't have a meaningful existence
	char* parentDir = malloc(9);
	char* slash = malloc(1);
	char* update = malloc(7);
	slash = "/";
	parentDir = "./";
	update = ".Update";
	
	char* updatePath = malloc(9+strlen(project)+7+1);
	
	strcpy(updatePath, parentDir);
	strcat(updatePath, project);
	strcat(updatePath, slash);
	strcat(updatePath, update);
	
	char* updateContents = readFromFile(updatePath);
	if(strcmp(updateContents, "FILE_DNE") != 0)
	{
		if(strcmp(updateContents, "EMPTY_FILE") != 0)
		{
			printf("ERROR: .Update exists and is not empty.\n");
			return -1;	
		}
	}
	
	//Checks that .Conflict doesn't exist
	char* conflictPath = malloc(9+7+strlen(project)+1);
	char* conflict = malloc(9);
	conflict = ".Conflict";
	
	strcpy(conflictPath, parentDir);
	strcat(conflictPath, project);
	strcat(conflictPath, slash);
	strcat(conflictPath, conflict);
	
	int conflictFd = open(conflictPath, O_RDONLY);
	if(conflictFd >= 0)
	{
		printf("ERROR: .Conflict exists.\n");
		close(conflictFd);
		return -1;
	}
	close(conflictFd);
	
	//Composes message for server
	char* clientcommand = malloc(6);
	char* delim = malloc(1);
	clientcommand = "commit";
	delim = ":";
	char* message = malloc(strlen(project)+6+1);
	strcpy(message, clientcommand);
	strcat(message, delim);
	strcat(message, project);
	
	//Connects to server, sends message
	int sockfd = connectToServer();
	if(sockfd < 0)
	{
		printf("ERROR: Could not connect to server.\n");
		return -1;
	}
	printf("Connection to server established.\n");
	
	//Reads server response 
	//Extracts information about upcoming data in from socket
	char* serverResponse = readUntilDelim(sockfd, ':');
	if(strcmp(serverResponse, "error") == 0)
	{
		printf("ERROR: Project does not exist.\n");
		return -1;
	}
	
	//On success, get number of bytes to read
	char* numBytesString = readUntilDelim(sockfd, ':');
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
	
	char* clientManifest = (char*)malloc(strlen(project)+9+7);
	char* manifest = (char*)malloc(9);
	manifest = ".Manifest";
	parentDir = "./";
	strcpy(clientManifest, parentDir);
	strcat(clientManifest, project);
	strcat(clientManifest, slash);
	strcat(clientManifest, manifest);
	
	//printf("manifest path %s\n", clientManifest);
	
	char* serverManifest = malloc(13);
	serverManifest = ".servManifest";
	
	int fd = open(serverManifest, O_RDWR|O_CREAT|O_APPEND, 0777);
	write(fd, manifestData, strlen(manifestData));
	close(fd);
	
	// char* serverManifest = malloc(20);
	// char* clientManifest = malloc(20);
	// serverManifest = "Client/.serverManifest";
	// clientManifest = "Client/.clientManifest";
	
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
		printf("ERROR: Please update local project.\n");
		return -1;
	}
	
	//free(cManifestVer);
	//free(sManifestVer);
	close(servManifestFd);
	
	char* commitPath = malloc(strlen(project)+7+10);
	char* commitFile = (char*)malloc(7);
	commitFile = ".Update";
	strcpy(commitPath, parentDir);
	strcat(commitPath, project);
	strcat(commitPath, slash);
	strcat(commitPath, commitFile);
	
	//printf(".commit path %s\n", commitPath);
	
	int commitFd = open(commitPath, O_RDWR|O_CREAT|O_APPEND, 0777);
	
	//Case 2: client and server manifests match
	while(1)
	{
		//Extracts line from client .Manifest and gets the project path
		char* currentClientLine = readUntilDelim(cliManifestFd, '\n');
		//printf("checkpoint1\n");

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
		char* filePath = malloc(strlen(parentDir)+strlen(project)+strlen(clientFile)+1);// proj1/example1.txt

		strcpy(filePath, parentDir);
		strcat(filePath, project);
		strcat(filePath, slash);
		strcat(filePath, clientFile);
												
		char* currentFileString = readFromFile(filePath);
	 	char* liveHash = hash(currentFileString);

		
		//Opens server Manifest and skips over version number
		servManifestFd = open(serverManifest, O_RDONLY);
		readUntilDelim(servManifestFd, '\n');
		
		//Loops through each line of server Manifest to check if project exists
		while(1)
		{
			//Extracts line from server .Manifest
			char* currentServerLine = readUntilDelim(servManifestFd, '\n');
			char* servHash = getHashFromLine(currentServerLine); //append with server hash???
			
			//Case 2.1: client project does not exist on server side -- must add code
			if(currentServerLine == NULL || strlen(currentServerLine) == 0)
			{
				//printf("checkpoint: client project does not exist on server side -- must add code\n");

				char action[400]; //malloc(strlen(clientFile)+liveHashSize+sizeof(char)*4); //+3 to account spaces and actionCode
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
				strcat(action, liveHash);
				strcat(action, newLine);
				writeLoop(commitFd, action, strlen(action));
				//printf("written action: %s\n", action);
				
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
				//printf("checkpoint:proj exists on both sides\n");

				//Case 2.2.1: server hash matches stored hash
				if(strcmp(servHash, storedCliHash) == 0)
				{
					//Case 2.2.1 extended: live hash does not match stored hash -- must modify code
					//printf("live hash: %s\n", liveHash);
					if(strcmp(liveHash, storedCliHash) != 0)
					{
						char action[400]; //malloc(strlen(clientFile)+liveHashSize+sizeof(char)*4); //+3 to account spaces and actionCode
						char* actionCode = malloc(sizeof(char)*2);
						char* space = malloc(sizeof(char)*1);
						char* newLine = malloc(sizeof(char)*1);
						actionCode = "M ";
						space = " ";
						newLine = "\n";
						
						//printf("serv hash: %s\n", servHash);
						
						strcpy(action, actionCode);
						strcat(action, clientFile);
						printf("%s\n", action);
	  				strcat(action, space);
						strcat(action, liveHash);
						strcat(action, newLine);
						writeLoop(commitFd, action, strlen(action));
						//printf("written action: %s\n", action);
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
						printf("ERROR: Failed to commit. Please synch client with repository before committing changes.\n");
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
		//printf("checkpoint: looping thru serv\n");

		char* currentServerLine = readUntilDelim(servManifestFd, '\n');
		
		//Closes fd and exits loop if no more lines
		if(currentServerLine == NULL || strlen(currentServerLine) == 0)
		{
			//printf("done loopin\n");
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
			//printf("checkpoint: looping thru cli\n");
			char* currentClientLine = readUntilDelim(cliManifestFd, '\n');
			char* clientFile = getFileFromLine(currentClientLine);
			
			//Case 2.3: server has files not on client side -- must delete code
			if(currentClientLine == NULL || strlen(currentClientLine) == 0)
			{
				//printf("checkpoint: must delete code\n");
				//char* storedCliHash = getHashFromLine(currentClientLine);
				char action[400]; 
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
				//printf("written action: %s\n", action);
				
				close(cliManifestFd);
				break;
			}
		
			//if project found on client side
			if(strcmp(clientFile,serverFile) == 0)
			{
				//printf("checkpoint: proj found\n");
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
	
	//Composes another message for server
	char* commitContent = readFromFile(commitPath);
	int commitSize = strlen(commitContent);
	char* commitSizeString = intToString(commitSize);
	char* message2 = malloc(strlen(commitSizeString)+strlen(commitContent));
	
	strcat(message2, commitSizeString);
	strcat(message2, delim);
	strcat(message2, commitContent);
	
	int newsockfd = connectToServer();
	if(newsockfd < 0)
		{
			printf("ERROR: Could not connect to server.\n");
			return -1;
		}
	printf("Connection to server established.\n");
	int finalsockfd = sendToServer(newsockfd, message);
	if(finalsockfd < 0)
	{
		printf("ERROR: Could not write to socket\n");
		return -1;
	}
	
	//Reads server response
	char* serverResponse2 = readUntilDelim(finalsockfd,':');
	if(strcmp(serverResponse2, "error") == 0)
	{
		printf("ERROR: Failed to commit.\n");
		remove(commitPath);
		close(newsockfd);
		close(finalsockfd);
		return -1;
	}

	close(newsockfd);
	close(finalsockfd);
	return 0;
}

//Pushes changes detailed in .Commit to server 
//Returns 0 on success, -1 on failure
int push(char* project) //Depends on server
{
	//Checks that .Commit exists
	char* parentDir = (char*)malloc(9);
	parentDir = "./";
	
	char* commitPath = malloc(strlen(project)+strlen(parentDir)+10);
	char* slash = (char*)malloc(1);
	char* dot = (char*)malloc(1);
	char* commitFile = (char*)malloc(7);
	commitFile = ".Commit";
	slash = "/";
	dot = ".";
	
	strcpy(commitPath, parentDir);
	strcat(commitPath, project);
	strcat(commitPath, slash);
	strcat(commitPath, commitFile);
	int commitFd = open(commitPath, O_RDONLY);
	if(commitFd < 0)
	{
		printf("ERROR: .Commit does not exist.\n");
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
		printf("ERROR: Could not connect to server.\n");
		return -1;
	}
	printf("Connection to server established.\n");
	int newsockfd = sendToServer(sockfd, message);
	if(newsockfd < 0)
	{
		printf("ERROR: Could not write to socket\n");
		return -1;
	}
	
	//Reads server response (which is a request for files)
	char* serverResponse = readUntilDelim(newsockfd,':');
	if(strcmp(serverResponse, "error") == 0)
	{
		printf("ERROR: Failed to push.\n");
	}
	
	file* fileLL = NULL;
	char* numFilesString = readUntilDelim(newsockfd,':');
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
	printf("Connection to server established.\n");
	transferOver(sockfd, fileLL, "push");
	
	char* response = readUntilDelim(sockfd, ':');
	
	//On success, get .Manifest from server
	if(strcmp(response, "success") == 0)
	{
		char* numBytesString = readUntilDelim(sockfd, ':');
		int numBytes = atoi(numBytesString);
		char* manifestContent = (char*)malloc(numBytes);
		status = 1;
	  readIn = 0;
		do
		{
	    status = read(sockfd, manifestContent+readIn, numBytes - readIn);
	    readIn += status;
	  } while(status > 0 && readIn < numBytes);
		
		//Builds .Manifest path
		char* manifest = malloc(8);
		manifest = ".Manifest";
		char* manifestPath = malloc(strlen(manifest)+strlen(parentDir)+strlen(project)+2);
		strcpy(manifestPath, parentDir);
		strcat(manifestPath, project);
		strcat(manifestPath, slash);
		strcat(manifestPath, manifest);
		
		remove(manifestPath);
		int manifestFd = open(manifestPath, O_RDWR|O_CREAT|O_APPEND, 0777);

		writeLoop(manifestFd, manifestContent, strlen(manifestContent));
		close(manifestFd);
	}
	else 
	{
		printf("ERROR: Failed to commit.\n");
	}
	
	remove(commitPath);
	
	return 0;
}

//Creates a project directory on the client only
//Returns 0 on success, -1 on failure
int create(char* project)
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
	
	//printf("checkpoint 1\n");
	
	//Connect and send to server
	int sockfd = connectToServer();
	if(sockfd < 0)
	{
		printf("ERROR: Could not connect to server.\n");
		return -1;
	}
	
	printf("Connection to server established.\n");
	int newsockfd = sendToServer(sockfd, message);
	close(sockfd);
	
	//sleep(3);
	
	//Reads response from server
	char* response = readUntilDelim(newsockfd, ':');
	
	close(newsockfd);
	
	//printf("response %s\n", response);
	
	//On success, creates directory
	if(strcmp(response, "error") != 0)
	{
		char* parentDir = (char*)malloc(9);
		parentDir = "./";
		char* projectPath = malloc(strlen(parentDir)+strlen(project)+1);
		strcpy(projectPath, parentDir);
		strcat(projectPath, project);
		int mkdirStatus = mkdir(projectPath, 0777); 
		if(mkdirStatus < 0)
		{
			printf("ERROR: Failed to create project.\n");
			return -1;
		}
		
		//Creates .Manifest in project, sets version to 0
		char* manifestPath = (char*)malloc(strlen(project)+12+7);
		char* slash = (char*)malloc(1);
		char* dot = (char*)malloc(1);
		char* manifest = (char*)malloc(9);
		manifest = ".Manifest";
		slash = "/";
		dot = ".";

		strcpy(manifestPath, parentDir);
		strcat(manifestPath, project);
		strcat(manifestPath, slash);
		strcat(manifestPath, manifest);
		
		int fd = open(manifestPath, O_RDWR|O_CREAT|O_APPEND, 0777);
		
		char* version = malloc(2);
		version = "0\n";
		write(fd, version, 2);
		
		//printf("manifestPath: %s\n", manifestPath);
		
		close(fd);
		close(mkdirStatus);
	}
	else
	{
		printf("ERROR: Failed to create project.\n");
		return -1;
	}
	return 0;
}

//Destroys a project on the server only
//Returns 0 on success, -1 on failure
int destroy(char* project)
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
		printf("ERROR: Could not connect to server.\n");
		return -1;
	}
	printf("Connection to server established.\n");
	int newsockfd = sendToServer(sockfd, message);
	close(sockfd);
	if(newsockfd < 0)
	{
		printf("ERROR: Could not write to socket\n");
		return -1;
	}
	
	char* serverResponse = readUntilDelim(newsockfd, ':');
	
	close(newsockfd);
	
	//On failure, print error
	if(strcmp(serverResponse, "error") == 0)
	{
		printf("ERROR: Could not destroy.\n");
		return -1;
	}
	
	return 0;
}

//Adds a file to a given project on the client only
//Returns 0 on success, -1 on failure
int add(char* project, char* file)
{
	//Builds file path from given info
	char* parentDir = (char*)malloc(9);
	char* path = malloc(strlen(project)+strlen(file)+2+9);
	char* slash = malloc(1);
	char* dot = malloc(1);
	slash = "/";
	dot = ".";
	parentDir = "./";
	
	strcpy(path, parentDir);
	strcat(path, project);
	strcat(path, slash);
	strcat(path, file);
	
	//Opens file
  int status = open(path, O_RDONLY);
	if(status < 0)
	{
		printf("ERROR: File does not exist on the client.\n");
		return -1;
	}
	close(status);
	
	//Opens project .Manifest
	char* manifestPath = malloc(strlen(project)+12+8);
	char* manifest = malloc(9);
	manifest = ".Manifest";
	strcpy(manifestPath, parentDir);
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
			strcat(toWrite, parentDir);
			strcat(toWrite, project);
			strcat(toWrite, slash);
			strcat(toWrite, file);
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

//Removes a file from a given project on the client only 
//Returns 0 on success, -1 on failure
int remove_(char* project, char* file)
{
	//Builds file path from given info
	char* parentDir = (char*)malloc(9);
	char* path = malloc(strlen(project)+strlen(file)+2);
	char* slash = malloc(1);
	char* dot = malloc(1);
	slash = "/";
	dot = ".";	
	parentDir = "./";
	
	strcpy(path, parentDir);
	strcat(path, project);
	strcat(path, slash);
	strcat(path, file);
	
	int status; 
  status = open(path, O_RDONLY);
	if(status < 0)
	{
		printf("ERROR: File does not exist.\n");
		return -1;
	}
	else
	{
		//Opens project .Manifest
		char* manifestPath = malloc(strlen(project)+12);
		char* manifest = malloc(9);
		manifest = ".Manifest";
		strcpy(manifestPath, parentDir);
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
		int beforeEntryLast = position-1; 
			
				
		//Create strings for first chunk and second chunk
		char* newManifestBefore = malloc(beforeEntryLast);
		char* newManifestAfter = strstr(nameInManifest, "\n");
		
		//Sets new strings char by char
		int j = 0, k = 0;
		for(j = 0; j < beforeEntryLast-1; j++)
		{
			newManifestBefore[j] = manifestString[j];
			//printf("newManifestBefore[j] = %c\n",newManifestBefore[j]);
		}
		
		//printf("before: %s\nafter: %s\n", newManifestBefore, newManifestAfter);
			
		//Replace .Manifest and write data
		remove(manifestPath);
		int manifestFd = open(manifestPath, O_RDWR|O_CREAT|O_APPEND, 0777);
		writeLoop(manifestFd, newManifestBefore, strlen(newManifestBefore));
		if(newManifestAfter!=NULL)
			writeLoop(manifestFd, newManifestAfter, strlen(newManifestAfter));
		
	}
	
	return 0;
}

//Gets the server's version of the given project
//Outputs all files in the project and their version numbers
int currentversion(char* project)
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
		printf("ERROR: Could not connect to server.\n");
		return -1;
	}
	printf("Connection to server established.\n");
	
	//Reads server response 
	
	//Extracts information about upcoming data in from socket
	char* serverResponse = readUntilDelim(sockfd, ':');
	
	if(strcmp(serverResponse, "error") == 0)
	{
		char* cause = readUntilDelim(sockfd, ' ');
		if(cause[0] == 'e')
		{
			printf("ERROR: Project is empty.\n");
		}
		else
		{
			printf("ERROR: Project does not exist.\n");
		}
		
		return -1;
	}
	
	//On success, get number of bytes to read
	char* numBytesString = readUntilDelim(sockfd, ':');
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

//Outputs a list of all successful pushes since the project's creation
//Returns 0 on success, -1 on failure
int history(char* project)
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
		printf("ERROR: Could not connect to server.\n");
		return -1;
	}
	printf("Connection to server established.\n");
	int newsockfd = sendToServer(sockfd, message);
	if(newsockfd < 0)
	{
		printf("ERROR: Could not write to socket.\n");
		return -1;
	}
	
	int status = 1, readIn = 0;

	char* serverResponse = readUntilDelim(newsockfd, ':');
	
	if(strcmp(serverResponse, "error") == 0)
	{
		printf("ERROR: Project does not exist on server.\n");
		return -1;
	}
	
	//Reads response
	char* numBytesString = readUntilDelim(newsockfd, ':');
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

//Reverts current version of projecct to a previous version number
//Returns 0 on success, -1 on failure
int rollback(char* project, char* version)
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
		printf("ERROR: Could not connect to server.\n");
		return -1;
	}
	printf("Connection to server established.\n");
	
	int newsockfd = sendToServer(sockfd, message);
	if(newsockfd < 0)
	{
		printf("ERROR: Could not write to socket\n");
		return -1;
	}
	
	//Reads response
	char* serverResponse = readUntilDelim(newsockfd, ':');
	if(strcmp(serverResponse, "error"))
	{
		char* errorCause = readUntilDelim(newsockfd, '\n');
		if(errorCause[0] == 'i')
		{
			printf("ERROR: Invalid version number.\n");
			return -1;
		}
		else
		{
			printf("ERROR: Project does not exist.\n");
			return -1;
		}
	}
	return 0;
}

//Sets global variables HOST and PORT using .Configure
//Returns 0 on success, -1 on failure
int setServerDetails()
{
	char* configure_string = readFromFile("./.Configure");
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

//Establishes a connection to the server
//Returns a socket fd on success, -1 on failure
int connectToServer()
{
	int sockfd = 0, cxn_status = 0; 

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
	if(sockfd < 0)
	{
		return -1;
	}
	
	struct hostent* host = gethostbyname(HOST);
	
	if(host == NULL)
	{
		return -1;
	}
	
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

//Sends a given message to an already-established socket 
//Returns a socket fd on success, -1 on failure
int sendToServer(int sockfd, char* message)
{
	writeLoop(sockfd,message,strlen(message));
	return sockfd;
}

//Reads entire file into string buffer
//Returns NULL if file does not exist, string otherwise
char* readFromFile(char* file)
{
    int fd = open(file, O_RDONLY);    // Returns -1 on failure, >0 on success
    // Fatal Error if file does not exist
    if(fd < 0){
        //printf("Fatal ERROR: File does not exist.\n");
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

//Hashes a given string using SHA1
//Returns the resulting code on success, NULL on failure
char* hash(char* data)
{
	int x = 0;
	size_t length = strlen(data);
	char* buffer = malloc(40);
	memset(buffer, '\0', 40);
	unsigned char hash[SHA_DIGEST_LENGTH];
	SHA1(data, length, hash);
	
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
	
	//printf("hash code: %s\n", buffer);
	//free(buffer);
	
	return buffer;
}

//Extracts stored hash code from .Manifest line
//Returns hash string on success, NULL on failure
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

	//printf("getHashFromLine buffer: %s\n", buffer);
	return buffer;
}

//Extracts project path from .Manifest line
//Returns full path on success, NULL on failure
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

//Extracts version number from .Manifest line
//Returns version string on success, NULL on failure
char* getVersionFromLine(char* line)
{
	int i = 0, end = 0, len = strlen(line);
	
	for(i = 0; i < len; i++)
	{
		if(line[i] == ' ' || line[i] == '\t') //space found
		{
			end = i;
			break;
		}
	}

	char* buffer = malloc(end);
	
	for(i = 0; i < end; i++)
	{
		buffer[i] = line[i];
	}

	//printf("getverfromline: %s\n", buffer);
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
		//sleep(3);
    do{
			//printf("in loop\n");
        status = read(fd, buffer+readIn, 1);
		// breaks and resets most recently read byte if  byte = delim
		if(buffer[readIn] == delim){
			buffer[readIn] = '\0';
			break;
		}
        readIn += status;
    } while(status > 0 && readIn < bufLen);
		
		//printf("result: %s\n", buffer);
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
	char* fullPath = (char*)malloc(strlen(name)+9);
	char* parentDir = malloc(9);
	parentDir = "./";
	strcpy(fullPath, parentDir);
	strcat(fullPath, name);
	char* fileString = readFromFile(fullPath);
	
	// FILE DNE - not exactly sure what to do or if this should be possible
	if(strcmp(fileString, "FILE_DNE") == 0){
		//printf("file dne\n");
		return fileLL;
	}
	file* temp = (file*)malloc(sizeof(file));
	temp->next = fileLL;
	temp->fileName = name;
	temp->nameLen = strlen(name);	
	
	// EMPTY FILE - set fileLen to 0, fileData = NULL
	if(strcmp(fileString, "EMPTY_FILE") == 0){
		//printf("empty file\n");
		temp->fileData = NULL;
		temp->fileLen = 0;
	}
	// NON-EMPTY FILE - set data and len as normal
	else{
		temp->fileLen = strlen(fileString);
		temp->fileData = fileString;
	}
	//printf("end of add checkpoint\n");
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