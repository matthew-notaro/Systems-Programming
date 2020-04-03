all: fileCompressor.c BST.o minHeap.o huff.o
	gcc -g fileCompressor.c BST.o minHeap.o huff.o -o fileCompressor

BST.o: BST.c
	gcc -c BST.c

minHeap.o: minHeap.c
	gcc -c minHeap.c

huff.o: huff.c
	gcc -c huff.c

clean:
	rm fileCompressor; rm BST.o; rm minHeap.o; rm huff.o