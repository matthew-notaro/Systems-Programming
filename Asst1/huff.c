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
        min1->freq += min2->freq;
        BSTNode* temp = min1->root;
        min1->root = (BSTNode*)malloc(sizeof(BSTNode));
        min1->root->token = min1->freq; // need to figure out how to convert int to string
        min1->root->left = temp;
        min1->root->right = min2->root;
        insertHeap(heap, min1);
        free(min2);
    }
    printf("total freq: %d\n", min1->freq);
    printBST(min1->root);
}