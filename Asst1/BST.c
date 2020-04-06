#include "BST.h"

int numUnique = 0, numTotal = 0; // numTotal probably not necessary but useful to compare to final freq

// Returns root of resultant BST after inserting/incrementing frequency of given word
BSTNode* insert(char* word, BSTNode *root){
    if(root == NULL){
        BSTNode* temp = (BSTNode*)malloc(sizeof(BSTNode));
        temp->freq = 1;
        temp->token = word;
        temp->left = NULL;
        temp->right = NULL;
        temp->huffCode = NULL;
        numUnique++;
        numTotal++;
        return temp;
    }
    else if(strcmp(root->token, word) == 0){
        root->freq++;
        numTotal++;
        return root;
    }
    else if(strcmp(root->token, word) > 0)
        root->left = insert(word, root->left);
    else
        root->right = insert(word, root->right);
    return root;
}

// Prints BST inorder
void printBST(BSTNode* root){
    if(root == NULL) return;
    printBST(root->left);
    printf("token: %s, freq: %d, code: %s\n", root->token, root->freq, root->huffCode);
    printBST(root->right);
}
// Frees BST
void freeBST(BSTNode* root){
    if(root == NULL) return;
    freeBST(root->left);
    /*  
    // need to free malloced codes
    if(strlen(root->huffCode) > 1){
        free(root->huffCode);
    }
    */
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
