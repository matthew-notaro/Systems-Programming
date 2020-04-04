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

char* readFromFile(char* file);
BSTNode* count_occs(char* file_string);
BSTNode* insert(char* word, BSTNode *root);

int build_tree(BSTNode* root);

int numUnique = 0, numTotal = 0, counter = 0;
char* escape = "%#$";

int main(int argc, char** argv){
  char* file = argv[1];
  char* file_string = readFromFile(file);
  // Checks for nonexistent file
  if(file_string == NULL){
      printf("Fatal Error: file \"%s\" does not exist", file);
      return -1;
  }

  printf("%s\n", file_string);
  BSTNode* root = count_occs(file_string); //call from somewhere else
  heapNode* tree = build_tree(root);
  return 0;
}

// Returns root of resultant BST after inserting/incrementing frequency of given word
BSTNode* insert(char* word, BSTNode *root){
    if(root == NULL){
        BSTNode* temp = (BSTNode*)malloc(sizeof(BSTNode));
        temp->freq = 1;
        temp->token = word;
        temp->left = NULL;
        temp->right = NULL;
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


    return root;
}

// Returns inorder sequence of a BST as an array of BSTNode pointers
BSTNode** treeToArr(BSTNode* root){
    BSTNode** arr = (BSTNode**)malloc(numUnique*sizeof(BSTNode*));
    counter = 0;
    return treeToArrHelper(root, arr);
}
// Helper so that counter can be set to 0 before computation
BSTNode** treeToArrHelper(BSTNode* root, BSTNode** arr){
    if(root == NULL) return NULL;
    treeToArrHelper(root->left, arr);
    arr[counter++] = root;
    treeToArrHelper(root->right, arr);
    return arr;
}

// Prints BST inorder
void printBST(BSTNode* root){
    if(root == NULL) return;
    printBST(root->left);
    printf("token: %s, freq: %d\n", root->token, root->freq);
    printBST(root->right);
}

void freeBST(BSTNode* root){
    if(root == NULL) return;
    freeBST(root->left);
    free(root);
    freeBST(root->right);
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
        printf("%s inserted\n", token);
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
  printBST(root);
  return root;
}

heapNode* build_tree(BSTNode* root)
{
  BSTNode** token_array = treeToArr(root);
  heapNode** token_heap = heapify(token_array);

  heapNode* root = NULL, left = NULL, right = NULL;

  while(getSize(token_heap) > 1)
  {
    left = deleteMin(token_heap);
    right = deleteMin(token_heap);
    // create new heap node with left and right children
  }

  root = deleteMin(token_heap);
  return root;
}

int build_tree(BSTNode* root)
{
  int fd = open("./HuffmanCodebook", O_RDWR|O_CREAT, 00600);
  
  return 0;
}
