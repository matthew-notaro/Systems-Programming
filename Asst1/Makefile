all: fileCompressor.c BST.o minHeap.o
	gcc fileCompressor.c BST.o minHeap.o -o fileCompressor

BST.o: BST.c
	gcc -c BST.c

minHeap.o: minHeap.c
	gcc -c minHeap.c

clean:
	rm fileCompressor; rm BST.o; rm minHeap.o