#ifndef BUILDBOOK_H
#define BUILDBOOK_H

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

void buildCodebook(char* path);
char* readFromFile(char* file){
BSTNode* stringToBST(char* fileString){
int writeBookToFile(int fd, BSTNode* huffTree){


#endif