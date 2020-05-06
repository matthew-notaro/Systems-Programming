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
#include <math.h>


int port = 0;

void checkout(int socketfd);
void updateCommit(int socketfd, char* command);
void upgrade(int socketfd);
void push(int socketfd);
void create(int socketfd);
void destroy(int socketfd);
void currentversion(int socketfd);
void history(int socketfd);
void rollback(int socketfd);

void* socketThread(void* sockfd);

typedef struct file{
	char* fileName;
	int nameLen;
	int fileLen;
	char* fileData;
	struct file* next;
} file;

typedef struct file2{
	char* fileName;
	int nameLen;
	int fileLen;
	char* fileData;
	struct file2* next;
	char* code;
	char* hash;
} file2;


struct file2* commitToLL(int commitfd);
char* readNBytes(int fd, int numBytes);
char* readUntilDelim(int fd, char delim);
char* readFromFile(char* file);
char* intToString(int num);
int writeLoop(int fd, char* str, int numBytes);
void transferOver(int sockfd, struct file* fileLL, char* command);
struct file *addDirToLL(struct file* fileLL, char *proj);
struct file* addFileToLL(struct file* fileLL, char* name);
void freeLL(struct file* head);
void freeLL2(struct file2* head);
char* hash(char* buffer);
struct file2* pushFileMatch(struct file2* head, char* fileName);
void downloadFile(char* fileName, char* fileData);
int extractNum(char* name);
int maxBackup(char* project);

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
// Function to be used
void* socketThread(void* sockvoidstar){
	printf("Connecting to a client\n");
	int sock = *((int*)sockvoidstar);
	int n;
	// Reads desired command from socket
	char* command = readUntilDelim(sock, ':');

	// CHECKOUT - Sends back entire project
	if(strcmp("checkout", command) == 0){
		checkout(sock);
	}

	// Sends back .Manifest of given project
	// Assumes request formatted as: <command>:<project>
	else if(strcmp("update", command) == 0 || strcmp("commit", command) == 0){
		updateCommit(sock, command);
	}
	
	// Reads in names of n files to be sent back to client
	// Protocol: <cmd>:<project>:<num>:<nameLen>:<fileName>...
	else if(strcmp("upgrade", command) == 0){
		upgrade(sock);
	}

	// PUSH - does some stuff
	else if(strcmp("push", command) == 0){
		push(sock);
	}
	
	// CREATE - creates new project's dir and .Manifest
	// Sends "success" or "error"
	else if(strcmp("create", command) == 0){
		create(sock);
	}

	// Reads project name then recursively deletes files then the dir
	else if(strcmp("destroy", command) == 0){
		destroy(sock);
	}

	// CURRENTVERSION - sends over all files w/ version numbers for a given project
	// Ignores the manifest version and all hash codes
	else if(strcmp("currentversion", command) == 0){
		currentversion(sock);
	}

	else if(strcmp("history", command)){
		history(sock);
	}
	else if(strcmp("rollback", command)){
		rollback(sock);
	}
	else{
		printf("failed\n");
		char* message = "error:invalid command";
		writeLoop(sock, message, strlen(message));
	}
	
	free(command);
	close(sock);
	printf("Disconnecting to a client\n");
	pthread_exit(NULL);
}


void checkout(int sock){
	char* message;
	char* project = readUntilDelim(sock, ':');
	
pthread_mutex_lock(&lock);
	file* fileLL = addDirToLL(fileLL, project);
pthread_mutex_unlock(&lock);
	
	// Transfer over or error if DNE
	if(fileLL != NULL){
		transferOver(sock, fileLL, "success");
	}
	else{
		message = "error:project does not exist";
		writeLoop(sock, message, strlen(message));
	}
	free(project);
	freeLL(fileLL);
}


// Sends back .Manifest of given project
// Assumes request formatted as: <command>:<project>
void updateCommit(int sock, char* command){
	// Format desired file name to <project>/.Manifest
	char* message;
	char* project = readUntilDelim(sock, '\n');
	char* manWithProj = (char*)malloc( (strlen(project)+10) * sizeof(char));
	strcpy(manWithProj, project);
	strcat(manWithProj, "/.Manifest");

	// Gets contents of project's .Manifest
pthread_mutex_lock(&lock);
	char* manContents = readFromFile(manWithProj);
pthread_mutex_unlock(&lock);

	// Writes error to socket
	if(strcmp(manContents, "FILE_DNE") == 0){
		message = "error:project does not exist";
		writeLoop(sock, message, strlen(message));
	}
	// Writes success:manLen:manContents to socket
	else{
		int manLen = strlen(manContents);
		char* manLenString = intToString(manLen);
		int manLenStringLen = strlen(manLenString);
		char* writeBuffer = (char*)malloc((manLen + manLenStringLen + 10) * sizeof(char));
		strcpy(writeBuffer, "success:");
		strcat(writeBuffer, manLenString);
		strcat(writeBuffer, ":");
		strcat(writeBuffer, manContents);
		writeLoop(sock, writeBuffer, strlen(writeBuffer));
		free(manLenString);
		free(writeBuffer);
	}
	free(manWithProj);


	// Must receive and save client's .Commit file
	// <fileLen>:<fileData>
	// Save file as .Commit<hash>
	if(strcmp(command, "commit")){
		char* fileLenString = readUntilDelim(sock, ':');
		int fileLen = atoi(fileLenString);
		char* fileData = readNBytes(sock, fileLen);

		// Calculates 40 char hash for given commit file
		char* hashCode = hash(fileData);

		// Formats name to include its hash and project - should be 47 chars long w/o project
		char* fileName = (char*)malloc(300 * sizeof(char));
		strcpy(fileName, project);
		strcat(fileName, "/.Commit");
		strcat(fileName, hashCode);

		// Tests to see if file already exists - should be 54 chars long
		// If exists, do nothing since files are identical
		// If doesn't exist, write new one
//pthread_mutex_lock(&lock);
		char* existBuffer = (char*)malloc(60 *sizeof(char));
		sprintf(existBuffer, "[ -f %s ]", fileName);
		// If file DNE, make new commit
		if(system(existBuffer) != 0){
			int commitfd = open(fileName, O_RDWR|O_CREAT|O_APPEND, 00600);
			writeLoop(commitfd, fileData, strlen(fileData));
			close(commitfd);
		}
//pthread_mutex_unlock(&lock);
		free(fileLenString);
		free(fileData);
		free(hashCode);
		free(fileName);
		free(existBuffer);
	}
	free(project);
}


// Reads in names of n files to be sent back to client
// Protocol: <cmd>:<project>:<num>:<nameLen>:<fileName>...
void upgrade(int sock){
	// Checks if project exists
	char* project = readUntilDelim(sock, ':');
	char projBuffer[300];
	char* message;
	file* fileLL;
	strcpy(projBuffer, "dir \0");
	strcat(projBuffer, project);
pthread_mutex_lock(&lock);
	// Project DNE
	if(system(projBuffer) != 0){
		message = "error:project does not exist";
		writeLoop(sock, message, strlen(message));
	}
	// Project exists
	// Sends protocol: success:<numFiles>:<nameLen>:<fileName><fileLen>:<fileData>...
	else{
		char* numFilesString = readUntilDelim(sock, ':');
		int numFiles = atoi(numFilesString);
		int i, nameLen;
		char* fileName, *nameLenString;
		// Reads nameLen to get fileName
		for(i = 0; i < numFiles; i++){
			nameLenString = readUntilDelim(sock, ':');
			nameLen = atoi(nameLenString);
			fileName = readNBytes(sock, nameLen);
			free(nameLenString);
			free(fileName);
			fileLL = addFileToLL(fileLL, fileName);

		}
		transferOver(sock, fileLL, "success");
		free(numFilesString);
	}
pthread_mutex_unlock(&lock);
	free(project);
}


// Receives client's current .Commit, checks if it had been stored
// If it had, request files from client
void push(int sock){

///////////////////////
// STEP 1
///////////////////////

	char* project = readUntilDelim(sock, ':');
	char* receivedHash = readUntilDelim(sock, ':'); //readNBytes(sock, 40);
	char* message;

	// Checks if project exists
	char projBuffer[300];
	strcpy(projBuffer, "dir \0");
	strcat(projBuffer, project);
	// Project DNE
	if(system(projBuffer) != 0){
		message = "error:project does not exist";
		writeLoop(sock, message, strlen(message));
		free(project);
		free(receivedHash);
		return;
	}

	// Checks if desired .Commit exists
	char* commitFile = (char*)malloc(50 *sizeof(char));
	strcpy(commitFile, ".Commit");
	strcat(commitFile, receivedHash);
	
pthread_mutex_lock(&lock);
	int commitfd = open(commitFile, O_RDONLY);
	if(commitfd < 0){
		message = "error:commit does not exist";
		writeLoop(sock, message, strlen(message));
		close(commitfd);
		free(commitFile);
		free(project);
		free(receivedHash);
		return;
	}
	//
	// .COMMIT EXISTS
	//
	message = "success:";
	writeLoop(sock, message, strlen(message));

	// For each line of .Commit, reads code, name, hash
	// Keeps looping until length of line read is 0
	// Reads 1st line of .Commit
	file2* fileLL = NULL;
	int numFiles = 0;
	char* line = readUntilDelim(commitfd, '\n');
	while(strlen(line) > 0){
		// mallocs space for new node
		file2* temp = (file2*)malloc(sizeof(file2));
		temp->code = (char*)malloc(5*sizeof(char));
		temp->hash = (char*)malloc(50*sizeof(char));
		temp->fileName = (char*)malloc(200*sizeof(char));
		temp->next = fileLL;

		// Scans everything into the new file2 node
		sscanf(line, "%s %s %s\n", temp->code, temp->fileName, temp->hash);
		free(line);
		// Incr numFiles if code is not D
		if(strcmp(temp->code, "D") != 0){
			numFiles++;
		}
		// Next line, move ptr up
		line = readUntilDelim(commitfd, '\n');
		fileLL = temp;
	}
	free(line);
	close(commitfd);

	// Write 1/2 message to socket
	char* numFilesString = intToString(numFiles);
	writeLoop(sock, numFilesString, strlen(numFilesString));
	free(numFilesString);
	file2* ptr = fileLL;
	char writeBuffer[250];
	// Requests files from client
	while(ptr != NULL){
		// Only request file on A/M
		if(strcmp(ptr->code, "D") != 0){
			memset(writeBuffer, '\0', 250);
			char* nameLenString = intToString(strlen(ptr->fileName));
			strcpy(writeBuffer, nameLenString);
			strcat(writeBuffer, ":");
			strcat(writeBuffer, ptr->fileName);
			writeLoop(sock, writeBuffer, strlen(writeBuffer));
			free(nameLenString);
		}
		ptr = ptr->next;
	}

///////////////////////
// STEP 2 (assuming reading returns right stuff)
///////////////////////


	// Gets message from client containing file contents
	char* command = readUntilDelim(sock, ':');
	// command should always be "push"
	if(strcmp(command, "push") != 0){
		message = "error:something bad happened";
		writeLoop(sock, message, strlen(message));
	}
	// Read in files from socket
	else{
		char* numFilesString = readUntilDelim(sock, ':');
		int numFiles = atoi(numFilesString);
		free(numFilesString);
		int i, nameLen;
		char* fileName, *nameLenString, *fileLenString;
		file2* ptr = fileLL;
		// Reads nameLen to get fileName
		for(i = 0; i < numFiles; i++){
			// Read nameLen, fileName
			nameLenString = readUntilDelim(sock, ':');
			nameLen = atoi(nameLenString);
			free(nameLenString);
			fileName = readNBytes(sock, nameLen);
			// Matches fileName to correct node in LL
			while(ptr != NULL){
				// If names match, read fileLen and fileData into node
				if(strcmp(ptr->fileName, fileName) == 0){
					fileLenString = readUntilDelim(sock, ':');
					ptr->fileLen = atoi(fileLenString);
					free(fileLenString);
					ptr->fileData = readNBytes(sock, ptr->fileLen);
					break;
				}
				ptr = ptr->next;
			}
		}

		// Updates .History - writes project version then copy and pastes .Commit<hash>

		char hisBuffer[300];
		strcpy(hisBuffer, project);
		strcat(hisBuffer, "/.History");
		int historyfd = open(hisBuffer, O_RDWR|O_APPEND);
		char manBuffer[300];
		strcpy(manBuffer, project);
		strcat(manBuffer, "/.Manifest");
	    int man1 = open(manBuffer, O_RDONLY);

		// Reads version # from old man, then write to .History
		char* line = readUntilDelim(man1, '\n');
		int manVersion = atoi(line);
		writeLoop(historyfd, line, strlen(line));
		writeLoop(historyfd, "\n", 1);
		free(line);

		// Writes all of commitfd to historyfd
		char* commitFileData = readFromFile(commitFile);
		writeLoop(historyfd, commitFileData, strlen(commitFileData));
		free(commitFileData);
		close(historyfd);

		// Deletes all .Commit files in project
		char cmdBuffer[300];
		strcpy(cmdBuffer, "rm ");
		strcat(cmdBuffer, project);
		strcat(cmdBuffer, "/.Commit*");
		system(cmdBuffer);

		//Gets max current version
		int max = maxBackup(project);
		char* maxString = intToString(max);

		// Creates tar file
		char tarFile[300];
		memset(cmdBuffer, '\0', 300);
		strcpy(cmdBuffer, "tar -zcvf ");
		memset(tarFile, '\0', 300);
		strcat(tarFile, maxString);
		strcat(tarFile, ".tar.gz ");
		strcat(cmdBuffer, tarFile);
		strcat(cmdBuffer, project);
		system(cmdBuffer);


		// move tar into ./project/.Archive
		memset(cmdBuffer, '\0', 300);
		strcpy(cmdBuffer, "mv ");
		strcat(cmdBuffer, tarFile);
		strcat(cmdBuffer, project);
		strcat(cmdBuffer, "/.Archive");
		// should be mv tar proj/.Archive
		system(cmdBuffer);


///////////////////////
// STEP 3
///////////////////////

		// 1st line of old man already opened and read
		char manBuffer2[300];
		strcpy(manBuffer2, manBuffer);
		strcat(manBuffer2, "2");
		int man2 = open(manBuffer2, O_RDWR|O_CREAT|O_APPEND, 00600);

		char* newVersionString = intToString(manVersion + 1);
		writeLoop(man2, newVersionString, strlen(newVersionString));
		free(line);
		free(newVersionString);

		int fileVer;
		char manFileName[300], hash[50];
		char lineBuffer[300];
		file2* matchNode;
		// For each line in old man, check if file name was in .Commit
		while(strlen(line) > 0){
			// Scans line of .Manifest for "version file hash"
			sscanf(line, "%d %s %s\n", &fileVer, manFileName, hash);

			// Iterates through fileLL to find a match
			matchNode = pushFileMatch(fileLL, manFileName);
			// NULL when fileName not in .Commit - just write to man unchanged
			if(matchNode == NULL){
				writeLoop(man2, line, strlen(line));
			}
			// !NULL - must check code
			else{
				// D - do not write anything to man2
				// download file from fileLL if M/A and write to man2
				if(strcmp(matchNode->code, "D") != 0){
					downloadFile(matchNode->fileName, matchNode->fileData);

					// Writes incr'ed file version to man2
					memset(lineBuffer, '\0', 300);
					sprintf(lineBuffer, "%d %s %s\n", ++fileVer, manFileName, matchNode->hash);
					writeLoop(man2, lineBuffer, strlen(lineBuffer));
				}
			}
			// Next line, move ptr up
			free(line);
			line = readUntilDelim(man1, '\n');
		}

		// Deletes original man, renames man2 to man, write success to client
		remove(manBuffer);
		rename(manBuffer2, manBuffer);
		message = "success";
		writeLoop(sock, message, strlen(message));
		
		// 
	}
pthread_mutex_unlock(&lock);

	free(project);
	free(receivedHash);
}

// Iterates through fileLL to find a match
// Returns node containing a match
file2* pushFileMatch(file2* head, char* fileName){
	file2* ptr = head;
	while(ptr != NULL){
		if(strcmp(ptr->fileName, fileName) == 0){
			return ptr;
		}
	}
	return NULL;
}


// TESTED - except for writing the file
// Given file path and intended file contents, opens new FD to path, making subdirectories if needed
// If one exists, deletes file found at path
void downloadFile(char* fileName, char* fileData){
	// First attempts to delete existing file to overwrite later
	char cmdBuffer[300];
    char first[300], second[300];
    char* next;
	memset(cmdBuffer, '\0', 300);
	strcpy(cmdBuffer, "rm ");
	strcat(cmdBuffer, fileName);
	system(cmdBuffer);
    
    // Reads off "." or project - both of which do not need to be checked if they exist
    memset(first, '\0', 300);
    strcpy(first, "mkdir ");
    memset(second, '\0', 300);
    next = strtok(fileName, "/");
    strcpy(second, next);

    // Makes missing directories in that path
    while(next != NULL){
        next = strtok(NULL, "/");
        if(next != NULL){
            strcpy(first, "mkdir ");
            strcat(first, second);
            strcat(second, "/");
            strcat(second, next);

            system(first); // first holds mkdir ./path
            printf("first: %s\n", first);
            printf("second: %s\n", second);
        }
    }

    // Now all dirs made, must write file
    int filefd = open(fileName, O_RDWR|O_CREAT|O_APPEND, 00600);
    writeLoop(filefd, fileData, strlen(fileData));
    close(filefd);
}


// NOT TESTED
int maxBackup(char* project){
  DIR* currentDir = opendir(project);
  struct dirent* currentThing = NULL;
  readdir(currentDir);
  readdir(currentDir);
  int max = 0;
  while((currentThing = readdir(currentDir)) != NULL){
    char buff[1024];
    if(currentThing->d_type == DT_REG){
      int curr = extractNum(currentThing->d_name);
	  if(curr > max){
		  max = curr;
	  }
    }
  }
  closedir(currentDir);
  return max;
}

// TESTED
// Extract all first consecutive numbers from name
int extractNum(char* name){
	int i;
	char* num = (char*)malloc(strlen(name) * sizeof(char));
	for(i = 0; i < strlen(name); i++){
		if(name[i] >= 0 || num[i] < 10){
			num[i] = name[i];
		}
		else{
			break;
		}
	}
	i = atoi(num);
	free(num);
	return i;
}


/*
The create command will fail if the project name already exists on the server or the client can not communicate
with the server. Otherwise, the server will create a project folder with the given name, initialize a .Manifest for it
and send it to the client. The client will set up a local version of the project folder in its current directory and
should place the .Manifest the server sent in it.
*/
void create(int sock){
	char* project = readUntilDelim(sock, ':');
	char* message;
	// Attempts to make new project's dir
pthread_mutex_lock(&lock);

	// Attempts to make new project's dir
	char cmdBuffer[300];
    memset(cmdBuffer, '\0', 300);
    strcpy(cmdBuffer, "mkdir ");
    strcat(cmdBuffer, project);
	if(system(cmdBuffer) != 0){
		message = "error:project already exists";
		writeLoop(sock, message, strlen(message));
	}
	else{
		// Creates empty .Manifest
		char* manWithProj = (char*)malloc( (strlen(project)+10) * sizeof(char));
		strcpy(manWithProj, project);
		strcat(manWithProj, "./Manifest");
		int manfd = open(manWithProj, O_RDWR|O_CREAT|O_APPEND, 00600);
		// Send success to client if creates project's .Manifest - which it always should
		if(manfd > 0){
			// Writes version "0" to new Manifest
			write(manfd, "0", 1);
			close(manfd);
		}
		free(manWithProj);

		// Creates empty .Archive directory
        memset(cmdBuffer, '\0', 300);
        strcpy(cmdBuffer, "mkdir ");
		strcat(cmdBuffer, project);
		strcat(cmdBuffer, "/.Archive");
		system(cmdBuffer); //project = project/.Archive
		
        // Creates empty .History file
        memset(cmdBuffer, '\0', 300);
        strcpy(cmdBuffer, "touch ");
        strcat(cmdBuffer, project);
        strcat(cmdBuffer, "/.History");
		system(cmdBuffer);

		// Sends success to client
		message = "success";
		writeLoop(sock, message, strlen(message));
	}
pthread_mutex_unlock(&lock);
	free(project);
}

// TESTED
// Reads project name then recursively deletes files then the dir
void destroy(int sock){
	char* project = readUntilDelim(sock, '\n');
	// Formats command to recursively delete project
	char* rmCommand = (char*)malloc( (strlen(project)+7) * sizeof(char));
	strcpy(rmCommand, "rm -r ");
	strcat(rmCommand, project);

	char* message;
	// system return 0 on success
	// Just assign message
	// rmCommand = rm -rf <project>
pthread_mutex_lock(&lock);
	if(system(rmCommand) != 0){
		message = "error:project does not exist";
	}
	else{
		message = "success";
	}
pthread_mutex_unlock(&lock);
	writeLoop(sock, message, strlen(message));
	free(rmCommand);
	free(project);
}


// TESTED
// Writes project's man w/o project version and file hash codes
void currentversion(int sock){
	// Formats .Manifest path to include project
	char* project = readUntilDelim(sock, '\n');
	char* manWithProj = (char*)malloc( (strlen(project)+10) * sizeof(char));
	char* message;
	strcpy(manWithProj, project);
	strcat(manWithProj, "./Manifest");
	
	// Attempts to open project's .Manifest if it exists
	int manfd = open(manWithProj, O_RDONLY);
	if(manfd > 0){
		// Does nothing with projVer
		char* projVer = readUntilDelim(manfd, '\n');
		free(projVer);

		// For each line of .Manifest, reads version # and name
		// Keeps looping until length of line read is 0
		char* line;
		int version, bytesWritten;
		char fileBuffer[200];
		char sockBuffer[200];
		int wroteToSock = 0;
pthread_mutex_lock(&lock);
		// Reads 1st line of .Manifest
		line = readUntilDelim(manfd, '\n');
		while(strlen(line) != 0){
			// Writes success and size of .Manifest to client before writing first line
			if(!wroteToSock){
				message = "success:";
				writeLoop(sock, message, strlen(message));

				// Writes length of manifest to socket
				struct stat *buffer = (struct stat*)malloc(sizeof(struct stat));
				stat(manWithProj, buffer);
				int manSize = buffer->st_size;
				char* manSizeString = intToString(manSize);
				writeLoop(sock, manSizeString, strlen(manSizeString));
				write(sock, ":", 1);
				free(manSizeString);
				free(buffer);
			}
			// Reset buffers
			memset(fileBuffer, '\0', 200);
			memset(sockBuffer, '\0', 200);
			
			wroteToSock = 1;
			sscanf(line, "%d %s\n", &version, fileBuffer);
//printf("%d. %s.\n", version, fileBuffer);
			sprintf(sockBuffer, "%d %s\n", version, fileBuffer);
			writeLoop(sock, sockBuffer, strlen(sockBuffer));
			free(line);
			line = readUntilDelim(manfd, '\n');
		}
pthread_mutex_unlock(&lock);
		free(line);

		// No files in project
		if(!wroteToSock){
			message = "error:empty project";
			writeLoop(sock, message, strlen(message));
		}
	}
	// Fails to open file
	else{
		message = "error:project does not exist";
		writeLoop(sock, message, strlen(message));
	}
	free(project);
	free(manWithProj);
}


// TESTED w/ hard coded project name and .History contents
// Writes contents of .History if it exists
void history(int sock){
	char* project = readUntilDelim(sock, ':');
	char* message;
	char historyBuff[300];
	strcpy(historyBuff, project);
	strcat(historyBuff, "./History");
	char* historyData = readFromFile(historyBuff);
	// Checks if project exists
	if(strcmp("FILE_DNE", historyData) == 0){
		message = "error:project does not exist";
		writeLoop(sock, message, strlen(message));
	}
	else{
		int hisLen = strlen(historyData);
		char* hisLenString = intToString(hisLen);
		char* writeBuffer = (char*)malloc((20 + hisLen) * sizeof(char));
		strcpy(writeBuffer, "success:");
		strcat(writeBuffer, hisLenString);
		strcat(writeBuffer, ":");
		strcat(writeBuffer, historyData);
		writeLoop(sock, writeBuffer, strlen(writeBuffer));

		free(hisLenString);
		free(writeBuffer);
	}
	free(historyData);
	free(project);
}


// TESTED - w/o fds
// Copies desired version tar file out of ./project/.Archive into ./
// Deletes project directory
// Extracts contents of NUM.tar.gz and deletes it
void rollback(int sock){
	char* project = readUntilDelim(sock, ':');
	char* version = readUntilDelim(sock, ':');
	char* message;
	char tarName[300];
	char cmdBuffer[300];
	memset(tarName, '\0', 300);
	memset(cmdBuffer, '\0', 300);

	// checks if directory exists:
	memset(cmdBuffer, '\0', 300);
	strcpy(cmdBuffer, "dir ");
	strcat(cmdBuffer, project);
	if(system(cmdBuffer) != 0){
		message = "error:p";
	}
	else{
        // Formats tar name
		strcpy(tarName, version);
		strcat(tarName, ".tar.gz");

		// mv ./project/.Archive/NUM.tar.gz ./
		strcpy(cmdBuffer, "mv ");
		strcat(cmdBuffer, project);
		strcat(cmdBuffer, "/.Archive/");
		strcat(cmdBuffer, tarName);
		strcat(cmdBuffer, " ./");
		// checks if NUM.tar.gz exists when trying to move
		if(system(cmdBuffer) == 0){
			// rm -rf project
			memset(cmdBuffer, '\0', 300);
			strcpy(cmdBuffer, "rm -rf ");
			strcat(cmdBuffer, project);
			system(cmdBuffer);

			// tar -zxvf NUM.tar.gz
			memset(cmdBuffer, '\0', 300);
			strcpy(cmdBuffer, "tar -zxvf ");
			strcat(cmdBuffer, tarName);
			system(cmdBuffer);

			// rm NUM.tar.gz
			memset(cmdBuffer, '\0', 300);
			strcpy(cmdBuffer, "rm ");
			strcat(cmdBuffer, tarName);
			system(cmdBuffer);

			message = "success";
		}
		// NUM.tar.gz DNE
		else{
			message = "error:i";
		}
	}
	writeLoop(sock, message, strlen(message));
	free(project);
	free(version);
}



// Hashes a given string and returns the code 
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
	
	printf("hash code: %s\n", buffer);
	//free(buffer);
	
	return buffer;
}

// TESTED
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

// TESTED
// Reads byte by byte from fd until the delim is found
// Delim is read then overwritten so next read will start after delim
// Returns string of at most bufLen bytes stored on the heap
// Max string len set to 200 - only reads numbers, names, lines of .Manifest
char* readUntilDelim(int fd, char delim){
	// Allocate 100 bytes - should be enough to store any file name or line of .Manifest
	int bufLen = 200;
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


// TESTED
// Reads entire file into string buffer
// Returns "FILE_DNE" if file does not exist, "EMPTY_FILE" if file empty, file contents otherwise
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


// NOT TESTED - pretty straightforward
// Given fileLL and file name, prepends new, populated file node to LL
// Returns given fileLL if file-to-be-added doesn't exist
file* addFileToLL(file* fileLL, char* name){
	char* fileString = readFromFile(name);
	// FILE DNE - checks if file exists
	if(strcmp(fileString, "FILE_DNE") == 0){
		return fileLL;
	}
	file* temp = (file*)malloc(sizeof(file));
	temp->next = fileLL;
	temp->fileName = name;
	temp->nameLen = strlen(name);
	
	// EMPTY FILE - set fileLen to 0, fileData = NULL
	// Should not be any empty files since all .Manifests have at least a version number
	if(strcmp(fileString, "EMPTY_FILE") == 0){
		temp->fileData = NULL;
		temp->fileLen = 0;
	}
	// NON-EMPTY FILE - set data and len as normal
	else{
		temp->fileData = fileString;
		temp->fileLen = strlen(fileString);
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
// <command>:<numFiles>:<nameLen>:<fileName><fileLen>:<fileData>
void transferOver(int sockfd, file* fileLL, char* command){
	// Trying to transfer empty LL
	if(fileLL == NULL){
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
	free(numFilesString);
	write(sockfd, ":", 1);

	// Reset loop to write each files' info to socket
	// <nameLen>:<fileName><fileLen>:<fileData>
	ptr = fileLL;
	while(ptr != NULL){
		
		char* nameLenString = intToString(ptr->nameLen);
		writeLoop(sockfd, nameLenString, strlen(nameLenString));
		free(nameLenString);
		write(sockfd, ":", 1);

		writeLoop(sockfd, ptr->fileName, ptr->nameLen);

		char* fileLenString = intToString(ptr->fileLen);
		writeLoop(sockfd, fileLenString, strlen(fileLenString));
		free(fileLenString);
		write(sockfd, ":", 1);

		writeLoop(sockfd, ptr->fileData, ptr->fileLen);
	}
	freeLL(fileLL);
}


// TESTED
// Loops write
int writeLoop(int fd, char* str, int numBytes){
    // IO Loop
    int status = 1;
    int writtenOut = 0;
    do{
        status = write(fd, str + writtenOut, numBytes - writtenOut);
        writtenOut += status;
    } while(status > 0 && writtenOut < numBytes);
	return writtenOut;
}

// TESTED
// Converts given int to string of appropriate length stored on heap
char* intToString(int num){
	char* itoabuf = (char*)malloc( (int)((ceil(log10(num))+1)) * sizeof(char));
    sprintf(itoabuf,"%d", num);
    return itoabuf;
}

// Frees fileLL
void freeLL(file* head){
	file* ptr;
	while(head != NULL){
		ptr = head;
		head = head->next;
		free(ptr->fileData);
		free(ptr->fileName);
		free(ptr);
	}
}

// Frees fileLL
void freeLL2(file2* head){
	file2* ptr;
	while(head != NULL){
		ptr = head;
		head = head->next;
		free(ptr->fileData);
		free(ptr->fileName);
		free(ptr->code);
		free(ptr->hash);
		free(ptr);
	}
}
