#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//create functions to test each command?

void main()
{
	system("./WTF create project1"); //Attempts to create without configuring
	
	system("./WTF configure host 42069"); //Successful configuration
	
	system("./WTF create project1"); //Successful creation
	system("./WTF create toBeDestroyed"); //Another successful creation
	system("./WTF create project1"); //Trying to create project that already exists
	
	system("./WTF destroy toBeDestroyed"); //Successful destroy
	system("./WTF destroy projectDNE"); //Attempts to destroy nonexistent project
	
	system("./WTF add project filename1"); //Successful add
	system("./WTF add project filename1"); //Attempts to add file that already exists
	system("./WTF add project toBeRemoved"); //Another successful add
	system("./WTF add projectDNE filename"); //Attempts to add to nonexistent project
	
	system("./WTF remove project toBeRemoved"); //Successful remove
	system("./WTF remove project filenameDNE"); //Attempts to remove nonexistent file
	system("./WTF remove projectDNE filename"); //Attempts to remove from nonexistent project
	
	system("./WTF commit project1");
	
	system("./WTF push project1");
	
	system("./WTF currentversion project1"); //Succ
	system("./WTF currentversion projectDNE"); //Proj DNE
	
	system("./WTF destroy project1"); //Successful 	
	system("./WTF checkout project1"); //Successful 
	
}    
