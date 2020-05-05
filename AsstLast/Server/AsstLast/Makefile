all: Client/WTFclient.c Server/WTFserver.c
	gcc -g Client/WTFclient.c -o WTF
	gcc -g Server/WTFserver.c -o WTFserver
	
test: WTF WTFserver WTFserver.c
	gcc -pthread WTFtest.c -o WTFTest
	
clean:
	rm WTF WTFserver