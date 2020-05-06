all: WTFclient.c WTFserver.c
	gcc -g -lm -lcrypto WTFclient.c -o WTF
	gcc -g -lm -lcrypto WTFserver.c -o WTFserver
	
client: WTFclient.c
	gcc -g -lm -lcrypto WTFclient.c -o WTF
	
server: WTFserver.c
	gcc -g -lm -lcrypto WTFserver.c -o WTFserver
	
sam: WTFclient.c
	gcc -g -lm WTFclient.c -o WTF
	gcc -g -lm WTFserver.c -o WTFserver
		
test:
	gcc -pthread WTFtest.c -o WTFtest
	
clean:
	rm -rf WTF WTFserver WTFtest WTF.dSYM