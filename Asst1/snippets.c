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

int compress(char* file, char* codebook);
int decompress(char* file, char* codebook);
char* getCodeFromBook(char* token, char* codebook);

int main(int argc, char** argv){

	// BSTNode* root = NULL;
  // root = insertCode(0, "100", "pls", root);
  // root = insertCode(0, "011", "word", root);
  // root = insertCode(0, "010", "REALLY", root);

  int status = decompress("../test.txt.hcz", "../HuffmanCodebook");

  return 0;
}

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

  //Reads given file into string
  char* file_string = readFromFile(file);

  //Build huffTree using given codebook
  BSTNode* huffTree = bookToBST(codebook);
	printBST(huffTree);
  BSTNode* ptr = NULL;

  int len = strlen(file_string);
/*
  for(i = 0; i < len; i++)
  {
    ptr = NULL;
    char currBit = file_string[i];
    do {
      if(currBit == '0')
      {
        ptr = huffTree->left;
      }
      else if(currBit == '1')
      {
        ptr = huffTree->right;
      }
      else if(isspace(currBit) == 0) //Not a 1, 0, or white space char
      {
        printf("Error: file is not formatted correctly.\n");
        return -1;
      }
    } while(ptr != NULL);

    if(ptr != NULL) // Leaf node found
    {
      tokenSize = strlen(huffTree->token)+1;
      write(fd, huffTree->token, tokenSize);
    }
  }*/
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
    if(isspace(currChar) != 0) //Delimiter found
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

      //If token is not empty, find its code in codebook and writes to file
      if(strlen(token) > 0)
      {
        code = getCodeFromBook(token, cb_string);
        codeSize = strlen(code)+1;
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
      codeSize = strlen(code)+1;
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
  char* currCharPtr = NULL;
  char* code = NULL;

  //Special case: token is a delimiter
  if(isspace(token[0]) != 0)
  {
    char* esc_token = NULL, *escape = NULL, *codebookPtr = NULL, *delim_check = NULL;
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
    delim_check = currCharPtr+strlen(esc_token);
    if(isspace(delim_check[0]) == 0)
    {
      while(isspace(delim_check[0]) == 0)
      {
        //Finds next token within codebook, checks until correct token found
        currCharPtr = strstr(delim_check, esc_token);
        delim_check = currCharPtr+strlen(esc_token);
      }
    }
    free(escape);
  }
  else
    currCharPtr = strstr(codebook, token); //Finds token within codebook

  //Token has been found
  if(currCharPtr != NULL)
  {
    //Skips tab so that currCharPtr points to last digit in code
    currCharPtr-=2;

    //Decrements pointer until it passes beginning of codebook
    while(currCharPtr[0] >= '0' && currCharPtr[0] <= '9')
    {
      currCharPtr--;
      count++;
    }

    //Sets currCharPtr to first digit of code
    currCharPtr++;

    //Mallocs space for code string
    code = malloc(count+1);
    for(i = 0; i < count; i++)
    {
      code[i] = currCharPtr[i];
    }
  }
  else
    printf("Error: code not found.\n");

  return code;
}
