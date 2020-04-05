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

char* readFromFile(char* file);
BSTNode* count_occs(char* file_string);
BSTNode* insert(char* word, BSTNode *root);
int build_codebook(int fd, BSTNode* huffTree);
int compress(char* file, char* codebook);
char* getCodeFromBook(char* token, char* codebook);

char* escape = "%#$";

int main(int argc, char** argv){
  /*char* file = argv[1];
  char* file_string = readFromFile(file);
  // Checks for nonexistent file
  if(file_string == NULL){
      printf("Fatal Error: file \"%s\" does not exist", file);
      return -1;
  }*/

  //printf("%s\n", file_string);
  //BSTNode* root = count_occs(file_string); //call from somewhere else

  BSTNode *huff = NULL;
  huff = insert("aa", huff);
  huff = insert("bb", huff);
  huff = insert("cc", huff);
  huff = insert("haaa", huff);
  huff = insert("y", huff);
  // huff = insert("g", huff);
  // huff = insert("g", huff);

  //int status = build_codebook(fd, huff);



  return 0;
}

int compress(char* file, char* codebook)
{
  //Mallocs memory to hold <filename>.hcz
  char* newFileName = malloc(strlen(file)+5);
  char* hcz = malloc(5);

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
      int token_cnt = 0;

      //Mallocs memory for delimiter and escape string
      char* delim = malloc(sizeof(char)*1);
      char* esc_text = malloc(sizeof(delim)+sizeof(escape)+1);
      char* code;

      if(token == NULL)
      {
        printf("Bad malloc\n");
        return -1;
      }
      memset(token, '\0', i-start+1);

      //Loops through file segment to extract token
      for(j = start; j < i; j++)
      {
        token[token_cnt] = file_string[j];
        token_cnt++;
      }

      //If token is not empty, inserts to BST
      if(strlen(token) > 0)
      {
        int tokenSize = strlen(token)+1;
        code = getCodeFromBook(token, cb_string);
        write(fd, code, sizeof(code));
      }

      //Increments starting point for next token
      start = i+1;

      //Convert delimiter char to a string
      delim = malloc(sizeof(char)+1);
      memset(delim, '\0', (sizeof(char)+1));
      delim[0] = currChar;

      //Get code
      code = getCodeFromBook(delim, cb_string);
      write(fd, code, sizeof(code)); //Check if sizeof(token) works
    }
  }

  return 0;
}

char* getCodeFromBook(char* token, char* codebook)
{
  char* test = malloc(5);
  test = "test\n";
  return test;

  /*
  int i = 0, count = 0;
  char* currChar;

  //Special case: token is a delimiter
  if(isspace(token[0] != 0))
  {
    char* esc_token, *escape;
    char* delim = malloc(sizeof(char));
    int size = 0;

    //Finds end of escape character
    while(isspace(codebook[0]) == 0)
    {
      //Starting from beginning, move pointer along until it reaches a space
      codebook++;
      count++;
    }

    //Extracts escape character
    escape = malloc(count+1);
    for(i = 0; i < count; i++)
    {
      escape[i] = codebook[i];
    }

    //Sets esc_token to be searched for in codebook
    if(token[0] == '\n')
    {
      size = strlen(escape)+2;
      esc_token = malloc(size);
      delim = "n";
      strcpy(esc_token, escape);
      strcat(esc_token, delim);
    }
    else if(token[0] == '\t')
    {
      size = strlen(escape)+2;
      esc_token = malloc(size);
      delim = "t";
      strcpy(esc_token, escape);
      strcat(esc_token, delim);
    }
    else if(token[0] == ' ')
      esc_token = escape;

    currChar = strstr(codebook, esc_token);
  }

  else
    currChar = strstr(codebook, token); //Finds token within codebook

  if(currChar != NULL)
  {
    //Skips tab so that currChar points to last digit in code
    currChar--;

    //Decrements pointer until it passes beginning of codebook
    //Check if this works if previous token in book is a digit --> should because \n
    while(isdigit(atoi(currChar)) != 0)
    {
      currChar--;
      count++;
    }

    //Sets currChar to first digit of code
    currChar++;

    //Mallocs
    char* code = malloc(count+1);
    for(i = 0; i < count; i++)
    {
      //code[i] = currChar+1;
    }
  }
    //then read characters until they are no longer ints
    return NULL;*/

}

/*
int decompress(struct dirent* file, *char codebook)
{
  int fd = open("./newfile", O_RDWR|O_CREAT|O_APPEND, 00600);
  char* file_name = file->name;
  char* file_string = readFromFile(file);

  //read num by num
  //hm.

}

int readCode()
{
  if(huffTree == NULL)
    return -1;

  if(huffTree->right)
  {
    //write(fd, "1", 2);
    build_codebook(fd, huffTree->right);
  }

  if(huffTree->left)
  {
    //write(fd, "0", 2);
    build_codebook(fd, huffTree->right);
  }
}
