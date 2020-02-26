#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

typedef struct Node
{
  char* data;
  struct Node* prev;
  struct Node* next;
} Node;

int insertionSort(void* toSort, int (*comparator)(void*, void*));
int quickSort(void* toSort, int (*comparator)(void*, void*));
char* init_things(char* file);
void extract_tokens(char* file_string);
Node* insert(char* num, Node* head);

int main(int argc, char** argv){
    // Command Line: exec file, sorting algo (-i / -q), file name
    if(argc != 3) {
      return -1;
    }

    char* algo = argv[1];
    char* file = argv[2];

    char* file_string = init_things(file);
    //printf("my file's content is %s\n", file_string);
    extract_tokens(file_string);

    return 0;
}

char* init_things(char* file)
{
  int fd = open(file, O_RDONLY);
  struct stat *buffer = malloc(sizeof(struct stat));
  stat(file, buffer);
  int size = buffer->st_size;
  printf("size %d\n", size);

  //if empty file

  char* file_buffer = malloc(size);
  memset(file_buffer, '\0', size);
  int status = 1;
  int readIn = 0;
  do{
      status = read(fd, file_buffer+readIn, size-readIn);
      //printf("status: %d\n", status);
      readIn += status;
      //printf("readIn: %d\n", readIn);
    } while(status > 0 && readIn < size);

  return file_buffer; //check --> warning?
}

Node* insert(char* token, Node* head)
{
  //case: insert to empty list
  if(head == NULL)
  {
    struct Node* newNode = (Node *)malloc(sizeof(Node));
    newNode->data = token;
    newNode->next = NULL;
    newNode->prev = NULL;
    head = newNode;
    return head;
  }

  //case: insert normally
  struct Node* newNode = (Node *)malloc(sizeof(Node));
  newNode->data = token;
  newNode->next = head;
  head->prev = newNode;
  head = newNode;
  return head;
}

void extract_tokens(char* file_string)
{
  struct Node* head = (Node *)malloc(sizeof(Node));
  int start = 0;
  int len = strlen(file_string);
  int i = 0;

  printf("extracting\n");

  for(i = 0; i < len; i++)
  {
    if(file_string[i] == ',')
    {
      //token found; create node
      printf("comma found\n");
      char substring[i-start+1];
      strncpy(substring, file_string+start, i-start);
      substring[i-start] = '\0';
      printf("substring: %s\n", substring);

      head = insert(substring, head);

      if(i+1 < len)
        start = i+1;
    }
  }

  //get last item
  if(start < len)
  {
    char substring[len-start];
    strncpy(substring, file_string+start, len-1-start);
    substring[len-1-start] = '\0';
    printf("substring: %s\n", substring);
    head = insert(substring, head);
  }
}
