#include "buildBook.h"

char* escape = "$420$";

// Returns updated BST after inserting file's contents
BSTNode* addToBook(char* path, BSTNode* oldBST){
  // Gets long string from contents of path
  char* fileString = readFromFile(path);
  if(fileString == NULL){
    return;
  }
  BSTNode* newBST = stringToBST(fileString, oldBST);     // fileString -> BST
  free(fileString);
  return newBST;
}

// Given BST with data from all files, creates codebook
void buildCodebook(BSTNode* finalBST){
    heapNode** heap = BSTToHeap(finalBST);          // BST -> heap
    printHeap(heap);
    huffEncode(heap);                               // heap -> huffman tree contained in heap[0]->root

    int huffFD = open("./HuffmanCodebook", O_WRONLY | O_CREAT | O_APPEND, 00600);
    write(huffFD, escape, strlen(escape));
    write(huffFD, "\n", 1);
    writeBookToFile(huffFD, heap[0]->root);

    //printBST(heap[0]->root);
    //freeHeap(heap);
    close(huffFD);
}

// Reads entire file into string buffer
// Returns NULL if file does not exist, string otherwise
char* readFromFile(char* file){
    int fd = open(file, O_RDONLY);    // Returns -1 on failure, >0 on success

    // Fatal Error if file does not exist
    if(fd < 0){
        printf("Fatal Error: File does not exist.\n");
        return NULL;
    }

    //Allocates memory for file buffer
    struct stat *buffer = (struct stat*)malloc(sizeof(struct stat));
    if(buffer == NULL){
        printf("Bad malloc\n");
        return NULL;
    }

    //Determines size of file
    stat(file, buffer);
    int buffer_size = buffer->st_size;

    // Warning: Empty file
    if(buffer_size == 0){
        printf("Warning: Empty file.\n");
    }

    //Mallocs and memsets file buffer for actual file contents
    char* file_buffer = (char*)malloc(buffer_size);
    if(file_buffer == NULL){
        printf("Bad malloc\n");
        return NULL;
    }
    memset(file_buffer, '\0', buffer_size);

    // IO Read Loop
    int status = 1;
    int readIn = 0;
    do{
        status = read(fd, file_buffer+readIn, buffer_size - readIn);
        readIn += status;
    } while(status > 0 && readIn < buffer_size);

    free(buffer);
    close(fd);
    return file_buffer;
}

//Counts occurrences of each unique token (including delimiters)
//Inserts token if new, increments occurrences otherwise
//Returns root of resulting BST on success, NULL on failure
BSTNode* stringToBST(char* fileString, BSTNode* root){
  int len = strlen(fileString);
  int start = 0, i = 0, j = 0, k = 0;

  //Loops through file string
  for(i = 0; i < len; i++){
    char currChar = fileString[i];

    //Extracts token
    if(isspace(currChar) != 0){ //Delimiter found
      //Mallocs space to hold substr from start to location of delimiter, +1 for '\0'
      char* token = (char*)malloc(i-start+1);
      if(token == NULL){
        printf("Bad malloc\n");
        return NULL;
      }
      memset(token, '\0', i-start+1);
      int token_cnt = 0;

      //Mallocs memory for delimiter and escape string
      char* delim = malloc(sizeof(char)*1);
      if(delim == NULL){
        printf("Bad malloc\n");
        return NULL;
      }
      char* esc_text = malloc(sizeof(delim)+sizeof(escape)+1);
      if(esc_text == NULL){
        printf("Bad malloc\n");
        return NULL;
      }

      //Loops through file segment to extract token
      for(j = start; j < i; j++){
        token[token_cnt] = fileString[j];
        token_cnt++;
      }

      //If token is not empty, inserts to BST
      if(strlen(token) > 0){
        root = insert(token, root);
      }

      //Increments starting point for next token
      start = i+1;

      //Inserts delimiter
      if(currChar == '\n'){
        delim = "n";
        strcpy(esc_text, escape);
        strcat(esc_text, delim);
        if(strlen(esc_text) > 0){
          root = insert(esc_text, root);
        }
      }
      else if(currChar == '\t'){
        delim = "t";
        strcpy(esc_text, escape);
        strcat(esc_text, delim);
        if(strlen(esc_text) > 0){
          root = insert(esc_text, root);
        }
      }
      else if(currChar == ' '){
        root = insert(escape, root);
      }

      free(token);
      free(delim);
      free(esc_text);
    }
  }
  //printBST(root);
  return root;
}

// Writes token and code from BST to file
void writeBookToFile(int fd, BSTNode* huffTree){
  if(huffTree == NULL) {
    return;
  }
  // Leaf Node - found token node
  if(huffTree->left == NULL){
    write(fd, huffTree->huffCode, strlen(huffTree->huffCode));
    write(fd, "\t", 1);
    write(fd, huffTree->token, strlen(huffTree->token));
    write(fd, "\n", 1);
    return;
  }
  writeBookToFile(fd, huffTree->left);
  writeBookToFile(fd, huffTree->right);
}


BSTNode* bookToBST(char* bookPath){
  char* bookString = readFromFile(bookPath);
  BSTNode* root = NULL;
  int len = strlen(bookString);
  int start = 0, i = 0, j = 0, k = 0, copyIndex;
  char* token, *code, *escapeString, *checkEscape;

  // Reads escape string then breaks
  for(i = 0; i < len; i++){
    if(bookString[i] == '\n'){
      escapeString = (char*)malloc(i * sizeof(char));
      memcpy(escapeString, bookString, i);
      i++;
      break;
    }
  }
  start = i;
  //Loops through bookString starting from 1st char of 1st code
  for(; i < len; i++){
    char currChar = bookString[i];

    if(currChar == '\t'){ // extract code that was just passed
      code = (char*)malloc((i - start) * sizeof(char));
      copyIndex = 0;
      // start catch up to i while copying
      while(start < i){
        code[copyIndex++] = bookString[start++];
      }
      // After copying, start is at index of \t, so start++ to have it point to index of 1st char of token
      start++;
    }
    else if(currChar == '\n'){ // extract token that was just passed
      token = (char*)malloc((i - start) * sizeof(char));
      copyIndex = 0;
      while(start < i){
        token[copyIndex++] = bookString[start++];
      }
      // After copying, start is at index of \t, so start++ to have it point to index of 1st char of token
      start++;
      // Check if special char
      checkEscape = strstr(token, escapeString);
      if(checkEscape != NULL){ // found special char
        if(strlen(token) == strlen(escapeString)){ // special char is a space
          free(token);
          token = (char*)malloc(sizeof(char));
          *token = ' ';
        }
        else if(strlen(token) == (strlen(escapeString) + 1)){ // special char is not a space
          char specialChar = token[strlen(escapeString)];
          free(token);
          token = (char*)malloc(sizeof(char));
          switch (specialChar){
              case 'n': *token = '\n';
              case 't': *token = '\t';
          }
        }
        else{
            printf("something bad happened extracting special char from escape char\n");
        }
      }

      root = insertCode(0, code, token, root);
    }
  }
  return root;
}

// Inserts huffcode and token into given BST, creating additional placeholder nodes along the way
// Returns resultant BST
BSTNode* insertCode(int index, char* codeString, char* token, BSTNode* root){
    // make new node if not made
    if(root == NULL){
      root = (BSTNode*)malloc(sizeof(BSTNode));
      root->token = "x";
      root->huffCode = NULL;
      root->right = NULL;
      root->left = NULL;
    }
    // Insert node in correct space
    if(index == strlen(codeString)){
      root->huffCode = codeString;
      root->token = token;
      return root;
    }
    // Recurse left/right depending on bit at current index
    if(codeString[index] == '0'){
      root->left = insertCode(index+1, codeString, token, root->left);
    }
    else if(codeString[index] == '1'){
      root->right = insertCode(index+1, codeString, token, root->right);
    }
    return root;
}
