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
int compress(char* file, char* codebook);
int decompress(char* file, char* codebook);
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

  // BSTNode *huff = NULL;
  // huff = insert("aa", huff);
  // huff = insert("bb", huff);
  // huff = insert("cc", huff);
  // huff = insert("haaa", huff);
  // huff = insert("y", huff);
  // huff = insert("g", huff);
  // huff = insert("g", huff);

  int status = compress("test.txt", "../HuffmanCodebook");

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
        return NULL;
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
    return file_buffer;
}

//Counts occurrences of each unique token (including delimiters)
//Inserts token if new, increments occurrences otherwise
//Returns root of resulting BST on success, NULL on failure
BSTNode* count_occs(char* file_string)
{
  BSTNode* root = NULL;
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
        return NULL;
      }
      int token_cnt = 0;

      //Mallocs memory for delimiter and escape string
      char* delim = malloc(sizeof(char)*1);
      if(delim == NULL){
        printf("Bad malloc\n");
        return NULL;
      }
      char* esc_text = malloc(sizeof(delim)+sizeof(escape)+1);
      if(esc_text == NULL){
        printf("Bad malloc\n");
        return NULL;
      }

      if(token == NULL)
      {
        printf("Bad malloc\n");
        return NULL;
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
        root = insert(token, root);
        //printf("%s inserted\n", token);
      }

      //Increments starting point for next token
      start = i+1;

      //Inserts delimiter
      if(currChar == '\n')
      {
        delim = "n";
        strcpy(esc_text, escape);
        strcat(esc_text, delim);

        if(strlen(esc_text) > 0)
          root = insert(esc_text, root);
      }
      else if(currChar == '\t')
      {
        delim = "t";
        strcpy(esc_text, escape);
        strcat(esc_text, delim);

        if(strlen(esc_text) > 0)
          root = insert(esc_text, root);
      }
      else if(currChar == ' ')
      {
        root = insert(escape, root);
      }
      free(token);
      free(delim);
      free(esc_text);
    }
  }
  //printBST(root);
  return root;
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
      char* esc_text = malloc(sizeof(delim)+sizeof(escape)+1);
      if(esc_text == NULL){
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
      free(esc_text);
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
/*
int decompress(char* file, *char codebook)
{
  //Mallocs memory to hold <filename>
  char* newFileName = malloc(strlen(file)-3);
  if(newFileName == NULL){
    printf("Bad malloc\n");
    return NULL;
  }
  newFileName = file;
  int i = 0, tokenSize = 0;

  //Opens new file
  int fd = open(newFileName, O_RDWR|O_CREAT|O_APPEND, 00600); //create new file

  //Reads given file into string
  char* file_string = readFromFile(file);
  //char* cb_string = readFromFile(codebook);

  //Build huffTree using given codebook
  Node* huffTree = build_tree(codebook);
  Node* ptr = NULL;

  int len = strlen(file_string);

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
  }
  return 0;
}*/
