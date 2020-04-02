#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>

typedef struct Node
{
  char* token;
  int code;
  int occurrences;
  struct Node* left;
  struct Node* right;
} Node;

Node* build_tree();
void build_codebook();
void compress();
void decompress();
void recurse(DIR* dd);
void read_file(char* file);

int b_flag = 0;
int c_flag = 0;
int d_flag = 0;

int main(int argc, char** argv)
{

  DIR* dd = opendir("./");

  recurse(dd);

  return 0;
}

void recurse(DIR* dd)
{

  readdir(dd);
  readdir(dd);
  struct dirent* currItem = NULL;

  do{
    currItem = readdir(dd);
    if(currItem->d_type == DT_REG)
    {
      //read_file
    }
    else if(currItem->d_type == DT_DIR)
    {
      recurse(currItem);
    }

  }while(currItem != NULL);

}

void read_file(char* file)
{
  //check flags, do things
}
