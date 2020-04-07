#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include "BST.h"
#include "minHeap.h"
#include "huff.h"
#include "buildBook.h"

void doOp(char* path);
void recursion(char* file);
void printCodeTree(BSTNode* root);
int compress(char* file, char* codebook);
int decompress(char* file, char* codebook);
char* getCodeFromBook(char* token, char* codebook);

int r_flag = 0;
char opFlag = '?';
char* codebook;
BSTNode* finalBST = NULL;

int main(int argc, char** argv){
  // min args - ./file -b <path>, max args - ./file -R -d <path> <codebook>
  if(argc < 3 || argc > 5){
    printf("Invalid arguments\n");
    return -1;
  }
  int i;
char* path;
  // Find flags and set appropriately
  for(i = 1; i < argc - 1; i++){
    if(strlen(argv[i]) == 2 && argv[i][0] == '-'){ // found a potential flag
      char flag = argv[i][1];
      if(flag == 'R' && !r_flag){ // flag is R and has not been set yet
        r_flag = 1;
      }
      else if((flag == 'b' || flag == 'c' || flag == 'd') && opFlag == '?'){ // flag is op and has not been set yet
        opFlag = flag;
        if(flag == 'b'){
          path = argv[argc-1];
        }
        else{
          path = argv[argc-2];
          codebook = argv[argc-1];
        }
      }
      else{ // bad flag if not b,c,d,R
        printf("Invalid flag\n");
        return -1;
      }
    }
    if(opFlag != 'b' && !(strcmp(codebook, "HuffmanCodebook") == 0 ||
      strcmp(codebook, "./HuffmanCodebook") == 0)){
      printf("Input correct book\n");
      return -1;
    }
    if(opFlag != 'b' && argc < 4){
      printf("Not enough arguments\n");
      return -1;
    }
    else if(opFlag == '?'){ // no opFlag found
      printf("Operation not specified\n");
      return -1;
    }
  }
  printf("OpFlag: %c, RFlag: %d\n", opFlag, r_flag);

  // If recursive flag set, then open given directory and apply operation to each file
  if(r_flag){
    recursion(path);
  }
  // If r not set, just do once
  else{
    doOp(path);
  }
  if(opFlag == 'b'){
    buildCodebook(finalBST);
  }

  //finalBST = addToBook(path, finalBST);
  //buildCodebook(finalBST);

  return 0;
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
  currentThing = readdir(currentDir);
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

void printCodeTree(BSTNode* root){
  if(root == NULL) return;
  printCodeTree(root->left);
  printf("code: %s\ttoken: %s\n", root->huffCode, root->token);
  printCodeTree(root->right);
}

// Performs b/c/d based on given operation determined by flag from command line
void doOp(char* path){
  switch(opFlag){
    case 'b': {finalBST = addToBook(path, finalBST); break;}
    case 'c': {compress(path, codebook); break;}
    case 'd': {decompress(path, codebook); break;}
  }
}

//Decompresses given file using given codebook
//Writes new file <filename> without .hcz extention
//Returns 0 on success, -1 on failure
int decompress(char* file, char* codebook)
{
  int sizeOfNewFile = strlen(file)-4, j = 0, i = 0, tokenSize = 0;

  //Extract name of new file from given file
  char* newFileName = malloc(sizeOfNewFile);
  for(j = 0; j < sizeOfNewFile; j++)
  {
    newFileName[j] = file[j];
  }

  //Opens new file
  int fd = open(newFileName, O_RDWR|O_CREAT|O_APPEND, 00600); //create new file
  if(fd < 0){
    printf("File does not exist.\n"); 
    return -1;
  }
  //Reads given file into string
  char* file_string = readFromFile(file);
  if(file_string == NULL) 
    return -1;
  //Builds Huffman Tree using given codebook
  BSTNode* huffTree = bookToBST(codebook);
	if(huffTree == NULL)
		printf("Error: Unable to interpret codebook.");

  BSTNode* ptr = huffTree;
  int len = strlen(file_string);
	char currBit;

	//Loops through each bit in compressed file
  for(i = 0; i < len; i++)
  {
		//Traverses Huffman Tree according to bit until leaf node is reachedß
		if(file_string[i] == '0')
       ptr = ptr->left;
  	else if(file_string[i] == '1')
       ptr = ptr->right;
		if(ptr->left == NULL && ptr->right == NULL)
		{
			//Token found; write to file
			if(isspace(ptr->token[0]) != 0 && ptr->token[1])
			{
				if(ptr->token[1] == 'n')
				{
					write(fd, "\n", 1);
					ptr = huffTree;
				}
				else if(ptr->token[1] == 't')
				{
					write(fd, "\t", 1);
					ptr = huffTree;
				}
			}
			else
			{
				tokenSize = strlen(ptr->token) + 1;
				//printf("%s\n", ptr->token);
				write(fd, ptr->token, tokenSize);
				ptr = huffTree;
			}
    }
  }
	free(newFileName);
	free(file_string);
	freeBST(huffTree);
  return 0;
}

//Compresses given file using given codebook
//Writes new file <filename>.hcz
//Returns 0 on success, -1 on failure
int compress(char* file, char* codebook)
{
  //Mallocs memory to hold <filename>.hcz
  char* newFileName = malloc(strlen(file)+5);
  if(newFileName == NULL){
    printf("Bad malloc\n");
    return -1;
  }
  char* hcz = malloc(5);
  hcz = ".hcz";
  strcat(newFileName, file);
  strcat(newFileName, hcz);

  //Opens new file
  int fd = open(newFileName, O_RDWR|O_CREAT|O_APPEND, 00600); //create new file
  if(fd < 0){
    printf("File does not exist.\n"); 
    return -1;
  }

  //Reads given file and codebook into strings
  char* file_string = readFromFile(file);
  char* cb_string = readFromFile(codebook);

  int len = strlen(file_string);
  int start = 0, i = 0, j = 0, k = 0;

  //Loops through file string
  for(i = 0; i < len; i++)
  {
    char currChar = file_string[i];

    //Extracts token
    if(isspace(currChar) != 0 || i == len-1) //Delimiter found or EOF
    {
      //Mallocs space to hold substr from start to location of delimiter, +1 for '\0'
      char* token = (char*)malloc(i-start+1);
      if(token == NULL){
        printf("Bad malloc\n");
        return -1;
      }
      memset(token, '\0', i-start+1);
      int token_cnt = 0;

      //Mallocs memory for delimiter and escape string
      char* delim = malloc(sizeof(char)*1);
      if(delim == NULL){
        printf("Bad malloc\n");
        return -1;
      }

      char* code = NULL;
      int codeSize = 0;

      //Loops through file segment to extract token
      for(j = start; j < i; j++)
      {
        token[token_cnt] = file_string[j];
        token_cnt++;
      }
			
			if(i == len-1)
				token[token_cnt] = file_string[len-1];
			
			printf("token: %s\n", token);

      //If token is not empty, find its code in codebook and writes to file
      if(strlen(token) > 0)
      {
        code = getCodeFromBook(token, cb_string);
        codeSize = strlen(code)+1;
				printf("code: %s\n", code);
        write(fd, code, codeSize);
      }

      //Increments starting point for next token
      start = i+1;

      //Convert delimiter char to a string
      delim = malloc(sizeof(char)+1);
      if(delim == NULL){
        printf("Bad malloc\n");
        return -1;
      }
      memset(delim, '\0', (sizeof(char)+1));
      delim[0] = currChar;

      //Gets delimiter's corresponding code and writes to file
      code = getCodeFromBook(delim, cb_string);
      codeSize = strlen(code);
			printf("code: %s\n", code);
      write(fd, code, codeSize);

      free(token);
      free(delim);
      free(code);
    }
  }
  free(newFileName);
  free(file_string);
  free(cb_string);

  return 0;
}

//Searches for given token in codebook and extracts corresponding code
//Returns string representation of code on success, NULL on failure
char* getCodeFromBook(char* token, char* codebook)
{
  int i = 0, count = 0;
  char* currCharPtr = NULL, *code = NULL, *check = NULL;

  //Special case: token is a delimiter
  if(isspace(token[0]) != 0)
  {
    char* esc_token = NULL, *escape = NULL, *codebookPtr = NULL;
    char* delim = malloc(sizeof(char));
    if(delim == NULL){
      printf("Bad malloc\n");
      return NULL;
    }
    int size = 0, delim_count = 0;
    codebookPtr = codebook;

    //Finds end of escape character
    while(isspace(codebookPtr[0]) == 0)
    {
      //Starting from beginning, move pointer along until it reaches a space
      codebookPtr++;
      delim_count++;
    }

    //Extracts escape character
    escape = malloc(delim_count+1);
    if(escape == NULL){
      printf("Bad malloc\n");
      return NULL;
    }
    for(i = 0; i < delim_count; i++)
    {
      escape[i] = codebook[i];
    }

    //Sets esc_token to the value to be searched for in the codebook
    if(token[0] == '\n')
    {
      size = strlen(escape)+2;
      esc_token = malloc(size);
      if(esc_token == NULL){
        printf("Bad malloc\n");
        return NULL;
      }
      delim = "n";
      strcpy(esc_token, escape);
      strcat(esc_token, delim);
      free(esc_token);
    }
    else if(token[0] == '\t')
    {
      size = strlen(escape)+2;
			esc_token = malloc(size);
      if(esc_token == NULL){
        printf("Bad malloc\n");
        return NULL;
      }
      delim = "t";
      strcpy(esc_token, escape);
      strcat(esc_token, delim);
      free(esc_token);
    }
    else if(token[0] == ' ')
      esc_token = escape;

    currCharPtr = strstr(codebookPtr, esc_token);

    //Checks delimiter
      //Since ' ' is represented by the escape character alone,
      //currCharPtr may be pointing to the wrong delimiter
    check = currCharPtr+strlen(esc_token);
    if(isspace(check[0]) == 0)
    {
      while(isspace(check[0]) == 0)
      {
        //Finds next token within codebook, checks until correct token found
        currCharPtr = strstr(check, esc_token);
        check = currCharPtr+strlen(esc_token);
      }
    }
    free(escape);
  }
  else
	{
		currCharPtr = strstr(codebook, token); //Finds token within codebook

		//Checks token to make sure it is not part of another token
    check = currCharPtr+strlen(token);
    if(isspace(check[0]) == 0)
    {
      while(isspace(check[0]) == 0)
      {
        //Finds next token within codebook, checks until correct token found
        currCharPtr = strstr(check, token);
        check = currCharPtr+strlen(token);
      }
    }
	}

	
	printf("currCharPtr: %s\n", currCharPtr);

  //Token has been found
  if(currCharPtr != NULL)
  {
    //Skips tab so that currCharPtr points to last digit in code
    currCharPtr-=2;

    //Decrements pointer until it passes beginning of code
    while(currCharPtr[0] >= '0' && currCharPtr[0] <= '9')
    {
      currCharPtr--;
      count++;
    }

    //Sets currCharPtr to first digit of code
    currCharPtr++;

    //Mallocs space for code string
    code = malloc(count+1);
		memset(code, '\0', count+1);
    for(i = 0; i < count; i++)
    {
      code[i] = currCharPtr[i];
    }
  }
  else
    printf("Error: code not found.\n");

  return code;
}
