#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "BST.h"
#include "minHeap.h"
/*
typedef struct Node{
  char* token;
  int code;
  int occurrences;
  struct Node* left;
  struct Node* right;
} Node;

Node* build_tree();
void build_codebook();
void compress();
void decompress();
void recurse(struct DIR* dd);
void read_file(char* file_name);
void count_occs(char* file_string);

char opFlag = '?';
int r_flag = 0;
char* path;
char* codebook;
*/
// test main for BST
int main(int argc, char** argv){
  BSTNode *root = NULL;
  root = insert("u", root);
  root = insert("g", root);
  root = insert("c", root);
  root = insert("c", root);
  root = insert("y", root);
  root = insert("g", root);
  root = insert("g", root);
  root = insert("g", root);
  root = insert("g", root);
  root = insert("d", root);
  BSTNode** arr = treeToArr(root); // must be freed in main file
  //printBSTArr(arr);
  heapNode** heap = heapify(arr);
  heapNode* min = deleteMin(heap);
  min = deleteMin(heap);
  heapNode* temp = (heapNode*)malloc(sizeof(heapNode));
  temp->freq = 7;
  temp->root = (BSTNode*)malloc(sizeof(BSTNode));
  temp->root->freq = 7;
  temp->root->token = " ";
  insertHeap(heap, temp);


  printHeap(heap);
  free(arr);
  free(heap);

  return 0;
}
  /*
int main(int argc, char** argv){
  // min args - ./file -b <path>, max args - ./file -R -d <path> <codebook>
  if(argc < 3 || argc > 5){
      return -1;
  }
  int i;
  for(i = 1; i < argc - 1; i++){
    if(strlen(argv[i]) == 2 && argv[i][0] == '-'){ // found a flag
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
    else if(opFlag == '?'){ // no opFlag found
      printf("Operation not specified\n");
      return -1;
    }
  }


  DIR* dd = opendir("./");

  //recurse(dd);

  return 0;
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

// Read entire file into string buffer
// Returns NULL if file does not exist, string otherwise
char* readFromFile(char* file_name)
{
    int fd = open(file_name, O_RDONLY);    // Returns -1 on failure, >0 on success
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
        readIn += status;
    } while(status > 0 && readIn < buffer_size);

    free(buffer);
    return file_buffer;
}

//Counts occurrences of each unique token
//Inserts token if new, increments occurrences otherwise
void count_occs(char* file_string)
{
  int i = 0;
  int start = 0;
  int len = strlen(file_string);

  for(i = 0; i < len; i++)
  {
    char currChar = file_string[0];
    if(isspace(currChar))
    {
      //get token
      //get delimiter
    }
  }

}

*/
