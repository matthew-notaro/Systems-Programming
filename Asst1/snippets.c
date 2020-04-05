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

  // BSTNode *huff = NULL;
  // huff = insert("aa", huff);
  // huff = insert("bb", huff);
  // huff = insert("cc", huff);
  // huff = insert("haaa", huff);
  // huff = insert("y", huff);
  // huff = insert("g", huff);
  // huff = insert("g", huff);

  int status = compress("test.txt", "HuffmanCodebook");

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
      int token_cnt = 0;

      //Mallocs memory for delimiter and escape string
      char* delim = malloc(sizeof(char)*1);
      char* esc_text = malloc(sizeof(delim)+sizeof(escape)+1);

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
    }
  }
  //printBST(root);
  return root;
}

int build_codebook(int fd, BSTNode* huffTree)
{
  if(huffTree == NULL)
    return -1;

  if(huffTree->right)
    build_codebook(fd, huffTree->right);

  if(huffTree->left)
    build_codebook(fd, huffTree->right);

  //Token found
  int tokenSize = strlen(huffTree->token) + 1;
  //int codeSize = sizeof();

  //write(fd, huffTree->huffCode, codeSize);
  write(fd, "\t", 2);
  write(fd, huffTree->token, tokenSize);
  write(fd, "\n", 2);

  return 0;
}

int compress(char* file, char* codebook)
{
  //Mallocs memory to hold <filename>.hcz
  char* newFileName = malloc(strlen(file)+5);
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
      int token_cnt = 0;

      //Mallocs memory for delimiter and escape string
      char* delim = malloc(sizeof(char)*1);
      char* esc_text = malloc(sizeof(delim)+sizeof(escape)+1);
      char* code = NULL;
      int codeSize = 0;

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
        code = getCodeFromBook(token, cb_string);
        int codeSize = strlen(code)+1;
        write(fd, code, codeSize);
      }

      //Increments starting point for next token
      start = i+1;

      //Convert delimiter char to a string
      delim = malloc(sizeof(char)+1);
      memset(delim, '\0', (sizeof(char)+1));
      delim[0] = currChar;

      //Get code
      code = getCodeFromBook(delim, cb_string);
      codeSize = strlen(code)+1;
      write(fd, code, codeSize);
    }
  }
  return 0;
}

char* getCodeFromBook(char* token, char* codebook)
{
  int i = 0, count = 0;
  char* currCharPtr = NULL;
  char* code = NULL;

  //Special case: token is a delimiter
  if(isspace(token[0]) != 0)
  {
    char* esc_token, *escape, *codebookPtr, *delim_check;
    char* delim = malloc(sizeof(char));
    int size = 0, delim_count = 0;
    codebookPtr = codebook;

    //Finds end of escape character
    while(isspace(codebookPtr[0]) == 0)
    {
      //Starting from beginning, move pointer along until it reaches a space
      codebookPtr++;
      delim_count++;
    }
    printf("codebookPtr: %s\n", codebookPtr);


    //Extracts escape character
    escape = malloc(delim_count+1);
    for(i = 0; i < delim_count; i++)
    {
      escape[i] = codebook[i];
    }

    printf("escape: %s\n", escape);

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
    {
      esc_token = escape;
    }

    currCharPtr = strstr(codebookPtr, esc_token);

    //Case: space is represented by escape character alone
    //currCharPtr may be pointing to a different delimiter
    delim_check = currCharPtr+strlen(esc_token);
    if(isspace(delim_check[0]) == 0)
    {
      while(isspace(delim_check[0]) == 0)
      {
        currCharPtr = strstr(delim_check, esc_token);
        delim_check = currCharPtr+strlen(esc_token);
      }
    }
  }

  else
    currCharPtr = strstr(codebook, token); //Finds token within codebook
    //printf("%s\n", currChar);

  printf("currCharPtr: %s\n", currCharPtr);

  if(currCharPtr != NULL)
  {
    //Skips tab so that currChar points to last digit in code
    //problem w/ pointer arith
    currCharPtr-=2;

    //Decrements pointer until it passes beginning of codebook
    //Check if this works if previous token in book is a digit --> should because \n
    while(currCharPtr[0] >= '0' && currCharPtr[0] <= '9')
    {
      currCharPtr--;
      count++;
    }

    //Sets currChar to first digit of code
    currCharPtr++;

    //Mallocs space for code string
    code = malloc(count+1);
    for(i = 0; i < count; i++)
    {
      code[i] = currCharPtr[i];
    }
    printf("code: %s\n", code);
    return code;
  }

  return code;
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
*/
