#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "BST.h"
#include "minHeap.h"
#include "huff.h"

char* readFromFile(char* file);
BSTNode* count_occs(char* file_string);
BSTNode* insert(char* word, BSTNode *root);
int build_codebook(int fd, BSTNode* huffTree);
int compress(struct dirent* file)

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

  int fd = open("./HuffmanCodebook", O_RDWR|O_CREAT|O_APPEND, 00600);

  int status = build_codebook(fd, huff);


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
  {
    //write(fd, "1", 2);
    build_codebook(fd, huffTree->right);
  }

  if(huffTree->left)
  {
    //write(fd, "0", 2);
    build_codebook(fd, huffTree->right);
  }

  //Token found
  int size = strlen(huffTree->token) + 1;

  write(fd, "temp", 5);
  write(fd, "\t", 2);
  write(fd, huffTree->token, size);
  write(fd, "\n", 2);

  return 0;
}

int compress(struct dirent* file, *char codebook)
{
  int fd = open("./newfile", O_RDWR|O_CREAT|O_APPEND, 00600);
  char* file_name = file->name;
  char* file_string = readFromFile(file);

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
        //getCode(token, codebook);
        //write to new file
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
        //getCode(esc_token, huffTree);
        //write to new file
      }
      else if(currChar == '\t')
      {
        delim = "t";
        strcpy(esc_text, escape);
        strcat(esc_text, delim);

        if(strlen(esc_text) > 0)
        //getCode(esc_text, codebook);
        //write to new file
      }
      else if(currChar == ' ')
      {
        //getCode(escape, codebook);
        //write to new file
      }
    }
  }
}

int decompress(struct dirent* file, *char codebook)
{
  int fd = open("./newfile", O_RDWR|O_CREAT|O_APPEND, 00600);
  char* file_name = file->name;
  char* file_string = readFromFile(file);

  //read one bit at a time

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

char* getCodeFromBook(char* token, char* codebook)
{
  int fd = open("codebook");
  int cb_string = readFromFile(codebook);
  if(strstr(codebook, word) != NULL)
  {
    //backtrack until new line?
    //then read characters until they are no longer ints
    //return code
  }
}

void recurse(DIR* dd)
{
  readdir(dd);
  readdir(dd);
  struct dirent* currItem = NULL;

  do{
    currItem = readdir(dd);
    if(currItem->d_type == DT_REG)
    {
      //readFromFile
    }
    else if(currItem->d_type == DT_DIR)
    {
      recurse(currItem);
    }

  }while(currItem != NULL);

}
