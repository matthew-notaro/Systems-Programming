#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>

typedef struct Node
{
  void* data;
  struct Node* prev;
  struct Node* next;
} Node;

int stringCmp(void* thing1, void* thing2);
int intCmp(void* thing1, void* thing2);

int insertionSort(void* head, int (*comparator)(void*, void*));
int quickSort(void* head, int (*comparator)(void*, void*));

Node* recursiveQuickSort(Node* left, Node* right, int (*comparator)(void*, void*));
Node* partition(Node* left, Node* right, int (*comparator)(void*, void*));

char* readFromFile(char* file);
Node* extractAndBuild(char* file_string);
Node* insert(void* num, Node* head);

void freeLL(Node* head);

void printLL(Node* head); // Useful function to print out LL

// Keep track of number of nodes in linked list
int size = 0;
int isIntFile = 0;

int main(int argc, char** argv){
    // Command Line: exec file, sorting algo (-i / -q), file name
    if(argc != 3){
        printf("Fatal Error: expected two arguments, had one\n");
        return -1;
    }


    char* sortingAlgo = argv[1];


    char* file = argv[2];

    // Checks if sorting algo flag is correct
    if((stringCmp(sortingAlgo, "-i") != 0) && (stringCmp(sortingAlgo, "-q") != 0)){
        printf("Fatal Error: \"%s\" is not a valid sort flag\n", sortingAlgo);
        return -1;
    }


    // Reads
    char* file_string = readFromFile(file);
    // Checks for nonexistent file
    if(file_string == NULL){
        printf("Fatal Error: file \"%s\" does not exist", file);
        return -1;
    }

    Node* head = extractAndBuild(file_string);

    // Sorts
    int status= (stringCmp(sortingAlgo, "-i") == 0) ?
        (isIntFile ? insertionSort(head, intCmp) : insertionSort(head, stringCmp)) :
        (isIntFile ? quickSort(head, intCmp) : quickSort(head, stringCmp));

    //printf("intFile: %d\n", isIntFile);
    printLL(head);
    freeLL(head);
    return status;
}

// Read entire file into string buffer
// Returns NULL if file does not exist, string otherwise
char* readFromFile(char* file)
{
    int fd = open(file, O_RDONLY);    // Returns -1 on failure, >0 on success
    // Fatal Error if file does not exist
    if(fd < 0){
        printf("Fatal Error: File does not exist.\n");
        return NULL;
    }
    struct stat *buffer = malloc(sizeof(struct stat));
    if(buffer == NULL){
        printf("Bad malloc\n");
        return NULL;
    }
    stat(file, buffer);
    int buffer_size = buffer->st_size;
    // Warning: Empty file
    if(buffer_size == 0){
        printf("Warning: Empty file.\n");
    }
    // IO Read Loop
    char* file_buffer = (char*)malloc(buffer_size);
    if(file_buffer == NULL){
        printf("Bad malloc\n");
        return NULL;
    }
    memset(file_buffer, '\0', buffer_size);
    int status = 1;
    int readIn = 0;
    do{
        status = read(fd, file_buffer+readIn, buffer_size - readIn);
        //printf("status: %d\n", status);
        readIn += status;
        //printf("readIn: %d\n", readIn);
    } while(status > 0 && readIn < buffer_size);

    free(buffer);
    return file_buffer;
}

// Extract tokens from file string
// Returns head of linked list
Node* extractAndBuild(char* file_string)
{
    Node* head = NULL;
    int len = strlen(file_string);
    int start = 0, i = 0, j = 0, k = 0;

    // Checks if dealing with a string or int file
    while(i < len && i != ',')
    {
        if(isdigit(file_string[i])){
            isIntFile = 1;
            break;
        }
        else if(isalpha(file_string[i])){
            isIntFile = 0;
            break;
        }
        i++;
    }

    int emptyFile = 1;
    for(k = 0; k < len; k++){
        if(isspace(file_string[k]) == 0)
        {
            emptyFile = 0;
            break;
        }
    }

    if(emptyFile == 1){
      printf("Warning: Empty File.\n");
    }

    // Iterates through file_string
    for(i = 0; i < len; i++){
        //Token found; extract
        if(file_string[i] == ','){
            //Malloc space to hold substr from start to location of comma, +1 for '\0'
            char* substring = (char*)malloc(i-start+1);
            if(substring == NULL){
                printf("Bad malloc\n");
                return NULL;
            }
            memset(substring, '\0', i-start+1);
            //Loops though chars of substring, find non-white spaces
            int substr_cnt = 0;
            for(j = start; j < i; j++){
                if(!isspace(file_string[j])){
                    substring[substr_cnt] = file_string[j];
                    substr_cnt++;
                }
            }
            //Inserts into linked list
            //printf("try to insert: %s:\n", substring);
            head = insert(substring, head);
            //Increments starting point for next token
            if(i+1 < len)
                start = i+1;
        }
        if(i == len-1 && start < len-1){
            //Malloc space to hold substr from start to location of comma, +1 for '\0'
            char* substring = (char*)malloc(i-start+1);
            if(substring == NULL){
                printf("Bad malloc\n");
                return NULL;
            }
            memset(substring, '\0', i-start+1);
            //Loops though chars of substring, find non-white spaces
            int substr_cnt = 0;
            for(j = start; j <= i; j++){
                if(!isspace(file_string[j])){
                    substring[substr_cnt] = file_string[j];
                    substr_cnt++;
                }
            }
            //Inserts into linked list
            //printf("try to insert: %s:\n", substring);
            head = insert(substring, head);
            //Increments starting point for next token
            if(i+1 < len)
                start = i+1;
        }
    }
    free(file_string);
    return head;
}



// SORTS
// Implementation of insertion sort on LL
// Returns -1 on NULL LL, 0 on success
int insertionSort(void* toSort, int (*comparator)(void*, void*)){
    if(toSort == NULL){
        return -1;
    }
    Node* head = (Node*)toSort;
    Node* currNode;
    // Iterates through for every element in LL
    for(currNode = head; currNode != NULL; currNode = currNode->next){
        Node* ptr = currNode;                   // Creates ptr to compare previous elements
        void* currData = currNode->data;        // Saves currNode data so it's not lost when overwritten
        // Compares previous values to curr, if prev>curr then shift prev value right by one and continue
        while(ptr->prev != NULL && (*comparator)(ptr->prev->data, currData) > 0){
            ptr->data = ptr->prev->data;
            ptr = ptr->prev;
        }
        // Sets currData in appropriate order
        ptr->data = currData;
    }
    return 0;
}

// Implementation of quick sort on LL
// Returns -1 on NULL LL, 0 on success
int quickSort(void* toSort, int (*comparator)(void*, void*)){
    if(toSort == NULL){
        return -1;
    }
    Node* head = (Node*)toSort;

    // Finds last Node in LL
    Node* rear = head;
    while (rear != NULL && rear->next != NULL)
    {
      rear = rear->next;
    }
    head = recursiveQuickSort(head, rear, comparator);
    return 0;
}

Node* recursiveQuickSort(Node* left, Node* right, int (*comparator)(void*, void*))
{
  // Declares/initializes return value
  Node* result;

  // Compares values of left and right until values cross over
  if(right != NULL && left != NULL)
  {
    if(right != left && left != right->next)
    {
      // Puts first Node in correct position
      Node* part = partition(left, right, comparator);

      // Calls function on the partitioned lists
      if(left != part)
        {
          if(comparator(left->data, part->prev->data) == 0)
            result = recursiveQuickSort(left, part->prev->prev, comparator);
          else
            result = recursiveQuickSort(left, part->prev, comparator);
        }

        if(right != part)
        {
          if(comparator(part->next->data, right->data) == 0)
            result = recursiveQuickSort(part->next->next, right, comparator);
          else
            result = recursiveQuickSort(part->next, right, comparator);
        }
    }
  }
  return left;
}

// Partitions Linked List
Node* partition(Node* left, Node* right, int (*comparator)(void*, void*))
{
  // Sets pivot to first Node
  Node* pivot = left;

  Node* fromLeft = left, *fromRight = right;
  void* temp;

  Node* i, *j;
  do
  {
    //Finds first value from left larger than pivot
    for(i = fromLeft; i != right->next; i = i->next)
    {

      //If data of current Node is less than pivot, set value and break
      if(comparator(i->data, pivot->data) > 0)
      {
        fromLeft = i;
        break;
      }
    }

    // Finds first value from right smaller than pivot
    for(j = fromRight; j != left->prev; j = j->prev)
    {
      // Breaks if pointers "cross over"
      if(i == j)
        break;

      // If data of current Node is greater than pivot, set value and break
      if(comparator(j->data, pivot->data) <= 0)
      {
        fromRight = j;
        break;
      }
    }

    // Checks reason for break
    if(i != j)
    {
      // Swaps data of fromRight and fromLeft if pointers have not crossed
      temp = fromLeft->data;
      fromLeft->data = fromRight->data;
      fromRight->data = temp;
    }

  } while(i != j);

  // Confirms reason for break
  // Swaps data of pivot and Node before fromLeft; puts pivot in correct position
  if(i == j)
  {

    temp = pivot->data;
    pivot->data = fromLeft->prev->data;
    fromLeft->prev->data = temp;
  }

  // Returns pivot in new location
  return fromLeft->prev;

}

// Returns null on failed malloc, head of new LL otherwise
Node* insert(void* token, Node* head){
  Node* newNode = (Node *)malloc(sizeof(Node));
  if(newNode == NULL){
      printf("Bad malloc\n");
      return NULL;
  }
  newNode->data = token;
  newNode->next = head;
  newNode->prev = NULL;
  //case: linked old head's prev if it exists to new head
  if(head != NULL){
    head->prev = newNode;
  }
  return newNode;
}


//COMPARATORS
// Returns <0 if t1<t2, 0 if t1=t2, >0 if t1>t2
int stringCmp(void* thing1, void* thing2){
    char *string1 = (char*)thing1, *string2 = (char*)thing2;
    int i = 0;
    while((string1[i] != '\0') && (string1[i] == string2[i]))
        i++;
    return string1[i] - string2 [i];
}
// Returns <0 if t1<t2, 0 if t1=t2, >0 if t1>t2
int intCmp(void* thing1, void* thing2){
    return atoi((char*)thing1) - atoi((char*)thing2);
}


// Useful little function
void printLL(Node* head){
    while(head != NULL){
        if(isIntFile){
            printf("%d\n", atoi(head->data));
        }
        else{
            printf("%s\n", (char*)head->data);
        }
        head = head->next;
    }
}


void freeLL(Node* head){
	Node *prev;
	while(head != NULL){
		prev = head;
		head = head->next;
		free(prev->data);
		free(prev);
	}
}
