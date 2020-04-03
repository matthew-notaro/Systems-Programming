#include "BST.h"

int numUnique = 0, numTotal = 0, counter = 0;

// Returns root of resultant BST after inserting/incrementing frequency of given word
BSTNode* insert(char* word, BSTNode *root){
    if(root == NULL){
        BSTNode* temp = (BSTNode*)malloc(sizeof(BSTNode));
        temp->freq = 1;
        temp->token = word;
        numUnique++;
        numTotal++;        
        return temp;
    }
    if(strcmp(root->token, word) == 0){
        root->freq++;
        numTotal++;
        return root;
    }
    if(strcmp(root->token, word) > 0)
        root->left = insert(word, root->left);
    else
        root->right = insert(word, root->right);
}

// Returns inorder sequence of a BST as an array of BSTNode pointers
BSTNode** treeToArr(BSTNode* root){
    BSTNode** arr = (BSTNode**)malloc(numUnique*sizeof(BSTNode*));
    counter = 0;
    return treeToArrHelper(root, arr);
}
// Helper so that counter can be set to 0 before computation
BSTNode** treeToArrHelper(BSTNode* root, BSTNode** arr){
    if(root == NULL) return NULL;
    treeToArrHelper(root->left, arr);
    arr[counter++] = root;
    treeToArrHelper(root->right, arr);
    return arr;
}

// Prints BST inorder
void printBST(BSTNode* root){
    if(root == NULL) return;
    printBST(root->left);
    printf("token: %s, freq: %d\n", root->token, root->freq);
    printBST(root->right);
}
// Prints array of BSTNode pointers
void printBSTArr(BSTNode** arr){
    int i;
    for(i = 0; i < numUnique; i++)
        printf("token: %s, freq: %d\n", arr[i]->token, arr[i]->freq);
}
// Frees BST
void freeBST(BSTNode* root){
    if(root == NULL) return;
    freeBST(root->left);
    free(root);
    freeBST(root->right);
}
// Returns global var so that var is accessable to other files
int getNumUnique(){
    return numUnique;
}
int getNumTotal(){
    return numTotal;
}
