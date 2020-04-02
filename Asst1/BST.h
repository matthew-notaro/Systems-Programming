#ifndef BST_H
#define BST_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct BSTNode{
    struct BSTNode *left, *right;
    int freq;
    char *token;
} BSTNode;

BSTNode* insert(char* word, BSTNode *root);
BSTNode** treeToArr(BSTNode* root);
BSTNode** treeToArrHelper(BSTNode* root, BSTNode** arr);

void printBST(BSTNode* root);
void printBSTArr(BSTNode** arr);
void freeBST(BSTNode* root);
int getNumUnique();
int getNumTotal();

#endif