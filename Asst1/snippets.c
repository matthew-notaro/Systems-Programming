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

char* readFromFile(char* file_name);
void count_occs(char* file_string);
BSTNode* insert(char* word, BSTNode *root);

int numUnique = 0, numTotal = 0, counter = 0;

int main(int argc, char** argv){
  char* file_name = argv[1];
  char* file_string = readFromFile(file_name);
  printf("%s\n", file_string);
  count_occs(file_string);
  return 0;
}

// Returns root of resultant BST after inserting/incrementing frequency of given word
BSTNode* insert(char* word, BSTNode *root){
    if(root == NULL){
        BSTNode* temp = (BSTNode*)malloc(sizeof(BSTNode));
        temp->freq = 1;
        temp->token = word;
        numUnique++;
        numTotal++;
        return temp;
    }
    if(strcmp(root->token, word) == 0){
        root->freq++;
        numTotal++;
        return root;
    }
    if(strcmp(root->token, word) > 0)
        root->left = insert(word, root->left);
    else
        root->right = insert(word, root->right);
}

// Prints BST inorder
void printBST(BSTNode* root){
    if(root == NULL) return;
    printBST(root->left);
    printf("token: %s, freq: %d\n", root->token, root->freq);
    printBST(root->right);
}

// Read entire file into string buffer
// Returns NULL if file does not exist, string otherwise
char* readFromFile(char* file)
{
    int fd = open(file, O_RDONLY);    // Returns -1 on failure, >0 on success
    // Fatal Error if file does not exist
    if(fd < 0){
        printf("Fatal Error: File does not exist.\n");
        return NULL;
    }
    struct stat *buffer = malloc(sizeof(struct stat));
    if(buffer == NULL){
        printf("Bad malloc\n");
        return NULL;
    }
    stat(file, buffer);
    int buffer_size = buffer->st_size;
    // Warning: Empty file
    if(buffer_size == 0){
        printf("Warning: Empty file.\n");
    }
    // IO Read Loop
    char* file_buffer = (char*)malloc(buffer_size);
    if(file_buffer == NULL){
        printf("Bad malloc\n");
        return NULL;
    }
    memset(file_buffer, '\0', buffer_size);
    int status = 1;
    int readIn = 0;
    do{
        status = read(fd, file_buffer+readIn, buffer_size - readIn);
        //printf("status: %d\n", status);
        readIn += status;
        //printf("readIn: %d\n", readIn);
    } while(status > 0 && readIn < buffer_size);

    free(buffer);
    return file_buffer;
}

//Counts occurrences of each unique token
//Inserts token if new, increments occurrences otherwise
void count_occs(char* file_string)
{
  BSTNode* root = NULL;
  int len = strlen(file_string);
  int start = 0, i = 0, j = 0;

  //Loop through file string
  for(i = 0; i < len; i++)
  {
    char currChar = file_string[i];

    //Extract token
    if(isspace(currChar) != 0) //Delimiter found
    {
      //Malloc space to hold substr from start to location of delimiter, +1 for '\0'
      char* token = (char*)malloc(i-start+1);
      int token_cnt = 0;

      if(token == NULL)
      {
        printf("Bad malloc\n");
        return;
      }

      memset(token, '\0', i-start+1);

      //Loop through token
      for(j = start; j < i; j++)
      {
         token[token_cnt] = file_string[j];
         token_cnt++;
      }

      printf("%s\n", token);
      root = insert("one", root);
      printf("inserted\n");

      //Increments starting point for next token
      if(i+1 < len)
        start = i+1;
    }
    if(i == len-1 && start < len-1){
        char* token = (char*)malloc(i-start+1);
        if(token == NULL){
            printf("Bad malloc\n");
            return NULL;
        }
        memset(token, '\0', i-start+1);
        //Loops though chars of token, find non-white spaces
        int token_cnt = 0;
        for(j = start; j <= i; j++){
                token[token_cnt] = file_string[j];
                token_cnt++;
        }
        //Inserts into linked list
        //printf("try to insert: %s:\n", token);
        root = insert(token, root);
        //Increments starting point for next token
        if(i+1 < len)
            start = i+1;
    }
  }
  printBST(root);
}
