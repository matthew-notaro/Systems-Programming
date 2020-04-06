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
//printf("deleted: %s, %s\n", min1->root->token, min2->root->token);
        min1->freq += min2->freq;
        BSTNode* temp = min1->root;
        min1->root = (BSTNode*)malloc(sizeof(BSTNode));
        min1->root->token = "freq"; //+ min1->freq; // need to figure out how to convert int to string
        min1->root->left = temp;
        min1->root->right = min2->root;
        insertHeap(heap, min1);
        free(min2);
    }
    char* bitString0 = (char*)malloc(sizeof(char));
    bitString0 = "0\0";
    calculateCodes(heap[0]->root->left, bitString0);
    char* bitString1 = (char*)malloc(sizeof(char));
    bitString1 = "1\0";
    calculateCodes(heap[0]->root->right, bitString1);
}


void calculateCodes(BSTNode* root, char* bitString){
    if(root == NULL) return;
    if(root->left == NULL){
//printf("inserting code: %s\n", bitString);
        root->huffCode = bitString;
        return;
    }
//if(root->left != NULL)
    calculateCodes(root->left, appendBit(bitString, '0'));
//if(root->right != NULL)
    calculateCodes(root->right, appendBit(bitString, '1'));
}
char* appendBit(char* bitString, char bit){
    int oldLen = strlen(bitString);
    int newLen = oldLen + 1;
    char* newString = (char*)malloc((newLen+1) * sizeof(char));
    memset(newString, '\0', newLen+1);
    memcpy(newString, bitString, strlen(bitString));
    newString[oldLen] = bit;
//printf(".%s.(len=%d) + .%c. = .%s.(len=%d)\n", bitString, oldLen, bit, newString, newLen);
    return newString;
}


void printHuff(heapNode* huffTree){
    printf("total freq: %d\n", huffTree->freq);
    printBST(huffTree->root);
}
