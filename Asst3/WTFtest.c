#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) 
{
	system("make sam");
	printf("-- Compilation successful -- \n");
	
	system("./Client/WTF create project1");
	printf("-- Unsuccessful attempt to create project -- \n");
	
	system("./Client/WTF configure 127.0.0.1 42069");
	printf("-- Configuration successful -- \n");
	
	system("./Client/WTF create project1");
	system("./Client/WTF create toBeDestroyed");
	printf("-- Successful attempts to create projects -- \n");
	
	system("./Client/WTF create project1");
	printf("-- Unsuccessful attempt to create project that already exists -- \n");
	
	system("./Client/WTF add project1 file1");
	system("./Client/WTF add project1 file2");
	printf("-- Successful attempts to add files -- \n");
	
	system("./Client/WTF add project1 file1");
	printf("-- Unsuccessful attempt to add file that already exists -- \n");
	
	system("./Client/WTF add nonexistentProject file1");
	printf("-- Unsuccessful attempt to add to project that does not exist -- \n");
	
	system("./Client/WTF remove project1 file2");
	printf("-- Successful attempt to remove file -- \n");
	
	system("./Client/WTF remove nonexistentProject file1");
	printf("-- Unsuccessful attempt to remove file from project that does not exist -- \n");
	
	system("./Client/WTF remove project1 file2");
	printf("-- Unsuccessful attempt to remove file that does not exist -- \n");
	
	system("./Client/WTF commit project1");
	printf("-- Successful attempt to commit project -- \n");
	
	system("./Client/WTF push project1");
	system("./Client/WTF push toBeDestroyed");
	printf("-- Successful attempts to push projects -- \n");
	
	system("./Client/WTF destroy toBeDestroyed");
	printf("-- Successful attempt to destroy project -- \n");
	
	system("./Client/WTF destroy nonexistentProject");
	printf("-- Unsuccessful attempt to destroy project that does not exist -- \n");
	
	// system("./WTF currentversion project1"); //Succ
	// system("./WTF currentversion projectDNE"); //Proj DNE
	// 
	// system("./WTF destroy project1"); //Successful 	
	// system("./WTF checkout project1"); //Successful 
	return 0;
}    
