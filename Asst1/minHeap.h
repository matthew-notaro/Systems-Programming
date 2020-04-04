#ifndef MINHEAP_H
#define MINHEAP_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "BST.h"

typedef struct heapNode{
    int freq;
    BSTNode *root;
} heapNode;

heapNode** BSTToHeap(BSTNode *root);
void heapify(heapNode** heap, int n, int i);
void BSTToHeapHelper(BSTNode* root, heapNode** heap);
heapNode* deleteMin(heapNode** heap);
void insertHeap(heapNode** heap, heapNode* temp);
int getHeapSize();
void printHeap(heapNode** heap);
void freeHeap(heapNode** heap);

#endif