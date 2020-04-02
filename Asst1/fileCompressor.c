#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//#include <dirent.h>
#include "BST.h"
#include "minHeap.h"

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
void recurse(struct DIR*);
void read_file(char* file);

char opFlag = '?';
int r_flag = 0;
char* path;
char* codebook;

// test main for BST
int main(int argc, char** argv){ 
  BSTNode *root = NULL;
  root = insert("u", root);
  root = insert("u", root);
  root = insert("c", root);
  root = insert("c", root);
  root = insert("y", root);
  root = insert("g", root);
  root = insert("g", root);
  root = insert("g", root);
  root = insert("g", root);
  root = insert("d", root);
  printBST(root);
  BSTNode** arr = treeToArr(root); // must be freed in main file
  heapNode** heap = heapify(arr);
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

  recurse(dd);

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
      //read_file
    }
    else if(currItem->d_type == DT_DIR)
    {
      recurse(currItem);
    }

  }while(currItem != NULL);

}

void read_file(char* file)
{
  //check flags, do things
}

*/