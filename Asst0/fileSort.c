#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int insertionSort(void* toSort, int (*comparator)(void*, void*));
int quickSort(void* toSort, int (*comparator)(void*, void*));
int main(int argc, char** argv){
    // Command Line: exec file, sorting algo (-i / -q), file name
    if(argc != 3){
        return -1;
    }

    char* algo = argv[1];
    char* file = argv[2];

    int fd = open(file, "O_RDONLY");


    return 0;
}