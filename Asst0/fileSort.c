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

int stringCmp(void*, void*);
int intCmp(void*, void*);

int insertionSort(void*, int (*comparator)(void*, void*));
int quickSort(void*, int (*comparator)(void*, void*));

char* readFromFile(char* file);
Node* extractAndBuild(char* file_string);
Node* insert(void* num, Node* head);

void printLL(struct Node* head); // Useful function to print out LL


// Keep track of number of nodes in linked list
int size = 0;
int isIntFile = 0;

int main(int argc, char** argv){
    // Command Line: exec file, sorting algo (-i / -q), file name
    if(argc != 3){
        printf("Enter correct parameters\n");
        return -1;
    }

    char* sortingAlgo = argv[1];
    char* file = argv[2];

    // Reads
    char* file_string = readFromFile(file);
    Node* front = extractAndBuild(file_string);

    /*int sortSuccessful = 0;
    if(stringCmp(sortingAlgo, "-i")){
        if(isIntFile)
            sortSuccessful = insertionSort(head, intCmp);
        else
            sortSuccessful = insertionSort(head, stringCmp);
    }
    else{
        if(isIntFile)
            sortSuccessful = quickSort(head, intCmp);
        else
            sortSuccessful = quickSort(head, stringCmp);
    }
    return sortSuccessful;*/
    // Can replace 14 previous lines with following 1 statement
    return (stringCmp(sortingAlgo, "-i") == 0) ?
        (isIntFile ? insertionSort(front, intCmp) : insertionSort(front, stringCmp)) :
        (isIntFile ? quickSort(front, intCmp) : quickSort(front, stringCmp));
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
    stat(file, buffer);
    int size = buffer->st_size;
    // Warning: Empty file
    if(size == 0){
        printf("Warning: Empty file.\n");
    }
    // IO Read Loop
    char* file_buffer = (char*)malloc(size);
    memset(file_buffer, '\0', size);
    int status = 1;
    int readIn = 0;
    do{
        status = read(fd, file_buffer+readIn, size-readIn);
        //printf("status: %d\n", status);
        readIn += status;
        //printf("readIn: %d\n", readIn);
    } while(status > 0 && readIn < size);

    return file_buffer;
}

// Extract tokens from file string
// Returns head of linked list
Node* extractAndBuild(char* file_string)
{
    Node* head = (Node*)malloc(sizeof(Node));
    int len = strlen(file_string);
    int start = 0, i = 0, j = 0;

    // Checks if dealing with a string or int file
    for(i = 0; i < len; i++){
        if(isdigit(file_string[i])){
            isIntFile = 1;
        }
        else if(isalpha(file_string[i])){
            isIntFile = 0;
        }
    }

    // Iterates through file_string
    for(i = 0; i < len; i++){
        //Token found; extract
        if(file_string[i] == ',' || (i == len-1 && start < len-1)){
            //Malloc space to hold substr from start to location of comma, +1 for '\0'
            char* substring = (char*)malloc(i-start+1);
            memset(substring, '\0', i-start+1);
            //Loop though chars of substring, find non-white spaces
            int substr_cnt = 0;
            for(j = start; j < i; j++){
                if(!isspace(file_string[j])){
                    substring[substr_cnt] = file_string[j];
                    substr_cnt++;
                }
            }
            //Insert into linked list
            head = insert(substring, head);
            //Increment starting point for next token
            if(i+1 < len)
                start = i+1;

            free(substring);
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
    Node* front = (Node*)toSort;
    Node* currNode;
    // Iterates through for every element in LL
    for(currNode = front; currNode != NULL; currNode = currNode->next){
        Node* ptr = currNode;                   // Creates ptr to compare previous elements
        void* currData = currNode->data;        // Saves currNode data so it's not lost when overwritten
        // Compares previous values to curr, if prev>curr then shift prev value right by one and continue
        while(ptr->prev != NULL && (*comparator)(ptr->prev->data, currData) > 0){
            ptr->data = ptr->prev->data;
            ptr = ptr->prev;
        }
        // Sets currData in appropriate order
        ptr->data = currData;
        printLL(front);
    }
    return 0;
}


// Implementation of quick sort on LL
// Returns -1 on NULL LL, 0 on success
int quickSort(void* toSort, int (*comparator)(void*, void*)){
    if(toSort == NULL){
        return -1;
    }
    Node* front = (Node*)toSort;
    Node* currNode;

    // Iterates through for every element in LL
    for(currNode = front; currNode != NULL; currNode = currNode->next){
        Node* ptr = currNode;                   // Creates ptr to compare previous elements
        void* currData = currNode->data;        // Saves currNode data so it's not lost when overwritten
        // Compares previous values to curr, if prev>curr then shift prev value right by one and continue
        while(ptr->prev != NULL && (*comparator)(ptr->prev->data, currData) > 0){
            ptr->data = ptr->prev->data;
            ptr = ptr->prev;
        }
        // Sets currData in appropriate order
        ptr->data = currData;
        printLL(front);
    }


    return 0;
}



// Returns null on failed malloc, head of new LL otherwise
Node* insert(void* token, Node* head){
  struct Node* newNode = (Node *)malloc(sizeof(Node));
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
    return *(int*)thing1 - *(int*)thing2;
}


// Useful little function
void printLL(Node* head){
    while(head != NULL){
        printf("%s ", (char*)head->data);
        head = head->next;
    }
    printf("\n");
}
