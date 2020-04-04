#ifndef HUFF_H
#define HUFF_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "minHeap.h"
#include "BST.h"

void huffEncode(heapNode** heap);
void calculateCodes(BSTNode* root, char* bitString);
void printHuff(heapNode* huffTree);
char* appendBit(char* bitString, char bit);

#endif