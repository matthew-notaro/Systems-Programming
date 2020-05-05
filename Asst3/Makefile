all: Client/WTFclient.c Server/WTFserver.c
	gcc -g -lm -lcrypto ./Client/WTFclient.c -o ./Client/WTF
	gcc -g -lm -lcrypto ./Server/WTFserver.c -o ./Server/WTFserver
	
client: Client/WTFclient.c
	gcc -g -lm -lcrypto ./Client/WTFclient.c -o ./Client/WTF
	
server: Server/WTFserver.c
	gcc -g -lm -lcrypto ./Server/WTFserver.c -o ./Server/WTFserver
	
sam: Client/WTFclient.c
	gcc -g -lm ./Client/WTFclient.c -o ./Client/WTF
		
test:
	gcc -pthread WTFtest.c -o WTFtest
	
clean:
	rm -rf ./Client/WTF ./Server/WTFserver ./Client/WTF.dSYM