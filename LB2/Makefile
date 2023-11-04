CC = gcc
CFLAGS = -g

SRCS_SRV = server/server.c
SRCS_CLNT = client/client.c
SRCS_RSA = rsa/rsa.c

.PHONY: build run clean

build: 
	$(CC) server/server.c $(CFLAGS) -c -o server/server.o
	$(CC) client/client.c $(CFLAGS) -c -o client/client.o
	$(CC) rsa/rsa.c $(CFLAGS) -c -o rsa/rsa.o
	$(CC) net/net.c $(CFLAGS) -c -o net/net.o
	$(CC) $(CFLAGS) server/server.o net/net.o rsa/rsa.o -o server/server
	$(CC) $(CFLAGS) client/client.o net/net.o rsa/rsa.o -o client/client

run: build
	./server/server &
	./client/client
	
clean:
	rm server/*.o || rm client/*.o || rm rsa/*.o || rm net/*.o
