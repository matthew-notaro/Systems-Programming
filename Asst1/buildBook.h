#ifndef BUILDBOOK_H
#define BUILDBOOK_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include "BST.h"
#include "minHeap.h"
#include "huff.h"

void buildCodebook(char* path);
char* readFromFile(char* file);
BSTNode* stringToBST(char* fileString);
void writeBookToFile(int fd, BSTNode* huffTree);

BSTNode* bookToBST(char* bookPath);
BSTNode* insertCode(int index, char* codeString, char* token, BSTNode* root);


#endif
