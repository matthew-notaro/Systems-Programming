all: fileCompressor.c BST.o
	gcc fileCompressor.c BST.o -o fileCompressor

BST.o: BST.c
	gcc -c BST.c

clean:
	rm fileCompressor; rm BST.o