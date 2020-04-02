#include "minHeap.h"

// Returns heapified array of heapNodes from given array of BSTNodes
// Converts each element of arr to a heapNode
heapNode** heapify(BSTNode **arr){
    int len = sizeof(arr)/sizeof(arr[0]);
    heapNode** heap = (heapNode**)malloc(len*sizeof(heapNode*));
    int i;
    for(i = 0; i < len; i++){
        BSTNode *tempBST = (BSTNode*)malloc(sizeof(BSTNode));
        tempBST->freq = arr[i]->freq;
        tempBST->token = arr[i]->token;
//        tempBST->left = NULL;
//        tempBST->right = NULL;
        heapNode *temp = (heapNode*)malloc(sizeof(heapNode));
        temp->freq = arr[i]->freq;
        temp->root = tempBST;
        heap[i] = temp;
    }
    for(i = len/2 - 1; i >= 0; i--){
        heapifyHelper(heap, len, i);
    }
    return heap;
}

void heapifyHelper(heapNode** heap, int n, int i){
    int max = i, left = 2*i + 1, right = 2*i + 2;
    if(left < n && heap[left]->freq > heap[max]->freq)
        max = left;
    if(right < n && heap[right]->freq > heap[max]->freq)
        max = right;
    if(max != i){
        heapNode* temp = heap[i];
        heap[i] = heap[max];
        heap[max] = temp;
        heapifyHelper(heap, n, max);
    }
}

void printHeap(heapNode** heap){
    int i;
    for(i = 0; i < sizeof(heap)/sizeof(heap[0]); i++){
    /*  printf("freq: %d\n", heap[i]->freq);
        printf("\ttree: \n");
        printBST(heap[i]->root);
    */
        printf("freq: %d\n", heap[i]->freq);
    }
}