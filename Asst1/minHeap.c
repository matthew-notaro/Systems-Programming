#include "minHeap.h"

int currLen = 0;
int maxLen = 0;
// Returns heapified array of heapNodes from given array of BSTNodes
// Converts each element of arr to a heapNode
heapNode** BSTToHeap(BSTNode *root){
    maxLen = getNumUnique();
    if(maxLen == 0){
        printf("empty BST\n");
        return NULL;
    }
    heapNode** heap = (heapNode**)malloc(maxLen*sizeof(heapNode*));
    BSTToHeapHelper(root, heap);
    if(currLen != maxLen){
        printf("currLne != maxLen\n");
        return NULL;
    }
    //Heapifies each parent node up to the root
    int i;
    for(i = maxLen/2 - 1; i >= 0; i--){
        heapify(heap, maxLen, i);
    }
    return heap;
}
// Compares root to children, swaps if necessary, then recursively heapifies if swap
// n - length of arr, i - current parent
void heapify(heapNode** heap, int n, int i){
    int min = i, left = 2*i + 1, right = 2*i + 2;
    if(left < n && heap[left]->freq < heap[min]->freq)
        min = left;
    if(right < n && heap[right]->freq < heap[min]->freq)
        min = right;
    if(min != i){
        heapNode* temp = heap[i];
        heap[i] = heap[min];
        heap[min] = temp;
        heapify(heap, n, min);
    }
}


// Wraps each BSTNode in a heapNode
void BSTToHeapHelper(BSTNode* root, heapNode** heap){
    if(root == NULL) return;
    BSTToHeapHelper(root->left, heap);
    heap[currLen] = (heapNode*)malloc(sizeof(heapNode));
    heap[currLen]->freq = root->freq;
    heap[currLen]->root = (BSTNode*)malloc(sizeof(BSTNode));
    heap[currLen]->root->freq = root->freq;
    heap[currLen]->root->token = root->token;
    heap[currLen]->root->left = NULL;
    heap[currLen]->root->right = NULL;
    currLen++;
    BSTToHeapHelper(root->right, heap);
}

// Add node to be inserted to last available index, then heapifies up from insert node's parent
void insertHeap(heapNode** heap, heapNode* temp){
    if(heap == NULL){
        printf("heap is NULL\n");
        return;
    }
    if(currLen >= maxLen){ // error if nothing has been deleted
        printf("Error: trying to insert into a full heap\n");
        return;
    }
    heap[currLen] = temp;
    int parentIndex = (currLen-1) / 2;
    currLen++;
    // Compares and swaps parent with min of children up until root or no swap made
    for(; parentIndex >=0; parentIndex = (parentIndex-1) / 2){
        int min = parentIndex, left = 2*parentIndex + 1, right = 2*parentIndex + 2;
        if(left < currLen && heap[left]->freq < heap[min]->freq)
            min = left;
        if(right < currLen && heap[right]->freq < heap[min]->freq)
            min = right;
        if(min != parentIndex){
            heapNode* temp = heap[parentIndex];
            heap[parentIndex] = heap[min];
            heap[min] = temp;
        }
        else{ // stop if no swap occurred
            break;
        }
    }
}


// Returns min element of heap
// Moves last element in heap to top, then heapifies down
heapNode* deleteMin(heapNode** heap){
    if(heap == NULL){
        printf("heap is NULL\n");
        return NULL;
    }
    heapNode* min = heap[0];
    heap[0] = heap[currLen - 1];
    currLen--;
    heapify(heap, currLen, 0); // only call once on root since children guaranteed to be heaps
    return min;
}


int getHeapSize(){
    return currLen;
}

// Prints heap in order of array - does not necessarily print in order of freqs
void printHeap(heapNode** heap){
    if(heap == NULL){
        printf("heap is NULL\n");
        return;
    }
    int i;
    printf("currLen: %d\n", currLen);
    for(i = 0; i < currLen; i++){
        printf("freq: %d\n", heap[i]->freq);
        printBST(heap[i]->root);
    }
}
// Frees each heapNode's BST, each heapNode, and the heapNode array
void freeHeap(heapNode** heap){
    if(heap == NULL){
        printf("heap is NULL\n");
        return;
    }
    int i;
    for(i = 0; i < maxLen; i++){
        if(heap[i] != NULL){
            freeBST(heap[i]->root);
            free(heap[i]);
        }
    }
    free(heap);
}