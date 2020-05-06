all: WTFclient.c WTFserver.c
	gcc -g WTFclient.c -o WTF -lm -lcrypto
	gcc -g WTFserver.c -o WTFserver -lm -lcrypto -pthread
	
client: WTFclient.c
	gcc -g WTFclient.c -o WTF -lm -lcrypto
	
server: WTFserver.c
	gcc -g WTFserver.c -o WTFserver -lm -lcrypto -pthread
	
sam: WTFclient.c
	gcc -g WTFclient.c -o WTF -lm
		
test:
	gcc WTFtest.c -o WTFtest -pthread
	
clean:
	rm -rf WTF WTFserver WTFtest WTF.dSYM
