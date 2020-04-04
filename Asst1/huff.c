#include "huff.h"

void huffEncode(heapNode** heap){
    if(heap == NULL || getHeapSize() == 0){
        printf("cannot huffEncode\n");
        return;
    }
    heapNode *min1, *min2;
    while(getHeapSize() != 1){ // guaranteed at least 2 elements in heap
        min1 = deleteMin(heap);
        min2 = deleteMin(heap);
        printf("deleted: %s, %s\n", min1->root->token, min2->root->token);
        min1->freq += min2->freq;
        BSTNode* temp = min1->root;
        min1->root = (BSTNode*)malloc(sizeof(BSTNode));
        min1->root->token = "freq"; //+ min1->freq; // need to figure out how to convert int to string
        min1->root->left = temp;
        min1->root->right = min2->root;
        insertHeap(heap, min1);
        free(min2);
    }
    calculateCodes(heap[0]->root, "");
}

void calculateCodes(BSTNode* root, char* bitString){
    if(root == NULL) return;
    if(root->left == NULL){
        root->huffCode = bitString;
        return;
    }
    calculateCodes(root->left, appendBit(bitString, '0'));
    calculateCodes(root->right, appendBit(bitString, '1'));
}

char* appendBit(char* bitString, char bit){
    int oldLen = strlen(bitString);
    int newLen = oldLen + 1;
    char* newString = (char*)malloc(newLen * sizeof(char));

    printf("\tbit string: %s\n", bitString);
    printf("\tbit string len: %d\n", oldLen);
    printf("\tnew bit string len: %d\n", newLen);

    memcpy(newString, bitString, strlen(bitString));
    newString[strlen(bitString)] = bit;
    return newString;
}


void printHuff(heapNode* huffTree){
    printf("total freq: %d\n", huffTree->freq);
    printBST(huffTree->root);
}