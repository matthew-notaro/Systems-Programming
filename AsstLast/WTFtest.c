#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int clientDriver();

int main(int argc, char **argv)
{
	system("mkdir -p -m777 Client");
	system("mkdir -p -m777 Server");

	system("mv ./WTF ./Client/WTF");
	system("mv ./WTFserver ./Server/WTFserver");

	pid_t pid = fork();
	if (pid == -1)
	{
		printf("ERROR: Could not fork.\n");
		return -1;
	}
	else if (pid == 0)
	{
		clientDriver();
	}
	else
	{
		system("make server");
		chdir("./Server");
		system("./WTFserver 42069");
		
	}

	system("killall -SIGINT WTFserver");

	return 0;
}

int clientDriver()
{
	system("make client");
	chdir("./Client");
	
	system("./WTF create project1");
	printf("-- Unsuccessful attempt to create project -- \n");

	system("./WTF configure 127.0.0.1 42069");
	printf("-- Configuration successful -- \n");

	system("./WTF create project1");
	system("./WTF create toBeDestroyed");
	printf("-- Successful attempts to create projects -- \n");

	system("./WTF create project1");
	printf("-- Unsuccessful attempt to create project that already exists -- \n");

	system("echo 'File1 contents' > ./proj1/file1.txt");
	system("echo 'File2 contents' > ./proj1/file2.txt");

	system("./WTF add project1 file1.txt");
	system("./WTF add project1 file2.txt");
	printf("-- Successful attempts to add files -- \n");

	system("./WTF add project1 file1.txt");
	printf("-- Unsuccessful attempt to add file that already exists -- \n");

	system("./WTF add nonexistentProject file1.txt");
	printf("-- Unsuccessful attempt to add to project that does not exist -- \n");

	system("./WTF remove project1 file2.txt");
	printf("-- Successful attempt to remove file -- \n");

	system("./WTF remove nonexistentProject file1.txt");
	printf("-- Unsuccessful attempt to remove file from project that does not exist -- \n");

	system("./WTF remove project1 file2.txt");
	printf("-- Unsuccessful attempt to remove file that does not exist -- \n");

	system("./WTF commit project1");
	printf("-- Successful attempt to commit project -- \n");

	system("./WTF push project1");
	system("./WTF push toBeDestroyed");
	printf("-- Successful attempts to push projects -- \n");

	system("./WTF destroy toBeDestroyed");
	printf("-- Successful attempt to destroy project -- \n");

	system("./WTF destroy nonexistentProject");
	printf("-- Unsuccessful attempt to destroy project that does not exist -- \n");

	return 0;
}
