all: Client/WTFclient.c Server/WTFserver.c
	gcc -g Client/WTFclient.c -o WTF
	gcc -g Server/WTFserver.c -o WTFserver
	
client: Client/WTFclient.c
	gcc -g Client/WTFclient.c -o WTF
	
server: Server/WTFserver.c
	gcc -g Server/WTFserver.c -o WTFserver
	
test:
	gcc -pthread WTFtest.c -o WTFTest
	
clean:
	rm WTF WTFserver