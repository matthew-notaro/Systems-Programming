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
void printBST(BSTNode* root);
void freeBST(BSTNode* root);
int getNumUnique();
int getNumTotal();

#endif