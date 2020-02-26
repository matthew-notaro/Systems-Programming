#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int insertionSort(void* toSort, int (*comparator)(void*, void*));
int quickSort(void* toSort, int (*comparator)(void*, void*));
void init_things(char* file);

int main(int argc, char** argv){
    // Command Line: exec file, sorting algo (-i / -q), file name
    if(argc != 3){
        return -1;
    }

    char* algo = argv[1];
    char* file = argv[2];

    init_things(file);

    return 0;
}

void init_things(char* file)
{
  int fd = open(file, "O_RDONLY");
  struct stat *buffer = malloc(sizeof(struct stat);
  stat(file, buffer);
  int size = buffer->st_size;



}
