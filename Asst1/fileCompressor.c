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

void doOp();
void printCodeTree(BSTNode* root);

int r_flag = 0;
char opFlag = '?';
char* path;
char* codebook;
BSTNode* finalBST = NULL;
/*
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
  //BSTNode** arr = treeToArr(root); // must be freed in main file
  printBST(root);
  heapNode** heap = BSTToHeap(root);
  heapNode* min = deleteMin(heap);
  min = deleteMin(heap);
  heapNode* temp = (heapNode*)malloc(sizeof(heapNode));
  temp->freq = 7;
  temp->root = (BSTNode*)malloc(sizeof(BSTNode));
  temp->root->freq = 7;
  temp->root->token = " ";
  insertHeap(heap, temp);
  heapNode** heap = (heapNode**)malloc(6*sizeof(heapNode*));
  int i;
  for(i = 0; i < 6; i++){
    heap[i] = (heapNode*)malloc(sizeof(heapNode));
    heap[i]->root = (BSTNode*)malloc(sizeof(BSTNode));
    heap[i]->root->left = NULL;
    heap[i]->root->right = NULL;
  }
  heap[0]->freq = 5;
  heap[0]->root->freq = 5;
  heap[0]->root->token = "a";
  heap[1]->freq = 9;
  heap[1]->root->freq = 9;
  heap[1]->root->token = "dog";
  heap[2]->freq = 12;
  heap[2]->root->freq = 12;
  heap[2]->root->token = "cat";
  heap[3]->freq = 13;
  heap[3]->root->freq = 13;
  heap[3]->root->token = "button";
  heap[4]->freq = 16;
  heap[4]->root->freq = 16;
  heap[4]->root->token = "ball";
  heap[5]->freq = 45;
  heap[5]->root->freq = 45;
  heap[5]->root->token = "and";

  printHeap(heap);

  printf("Encoding:\n");
  huffEncode(heap);
  printHuff(heap[0]);
  //printBST(heap[0]->root);

  //free(arr);
  free(heap);

  return 0;
}
*/

// REAL MAIN
int main(int argc, char** argv){
  // min args - ./file -b <path>, max args - ./file -R -d <path> <codebook>
  if(argc < 3 || argc > 5){
    printf("Invalid arguments\n");
    return -1;
  }
  int i;
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
    else if(opFlag == '?'){ // no opFlag found
      printf("Operation not specified\n");
      return -1;
    }
  }
  printf("OpFlag: %c, RFlag: %d\n", opFlag, r_flag);
  /*
  // If recursive flag set, then open given directory and apply operation to each file
  if(r_flag){
    DIR* currentDir = openDir(path);
    if(currentDir == NULL){
      printf("Error: invalid path\n");
      return -1;
    }
    struct dirent* currentThing = NULL;
    readdir(currentDir);
    readdir(currentDir);
    currentThing = readdir(currentDir);
    while(currentThing != NULL){
      if(currentThing->d_type == DT_REG){
        doOp();
      }
      else if(currentThing->d_type == DT_DIR){
        // do nothing?
      }
      currentThing = readdir(currentDir);
    }
  }
  // If r not set, just do once
  else{
    doOp();
  }*/
  //if(opFlag = 'b'){
  //  buildCodebook(finalBST);
  //}

  finalBST = addToBook(path);
  buildCodebook(finalBST);

  return 0;
}

void printCodeTree(BSTNode* root){
  if(root == NULL) return;
  printCodeTree(root->left);
  printf("code: %s\ttoken: %s\n", root->huffCode, root->token);
  printCodeTree(root->right);
}


// Performs b/c/d based on given operation determined by flag from command line
void doOp(){
  switch(opFlag){
    case 'b': finalBST = addToBook(path);
    //case 'c': compress(path, codebook);
    //case 'd': decompress(path, codebook);
  }
}