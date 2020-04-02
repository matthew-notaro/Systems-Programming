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

heapNode** heapify(BSTNode **arr);
void heapifyHelper(heapNode** heap, int n, int i);
void printHeap(heapNode** heap);

#endif