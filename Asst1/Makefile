all: snippets.c BST.o minHeap.o huff.o buildBook.o
	gcc -g snippets.c BST.o minHeap.o huff.o buildBook.o -o snippets

BST.o: BST.c
	gcc -c BST.c

minHeap.o: minHeap.c
	gcc -c minHeap.c

huff.o: huff.c
	gcc -c huff.c

buildBook.o: buildBook.c
	gcc -c buildBook.c

clean:
	rm fileCompressor; rm BST.o; rm minHeap.o; rm huff.o; rm buildbook.o; rm HuffmanCodebook
