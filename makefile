CC = gcc
CFLAGS = -Wall

all: server client

server: server.c gameF6.h cmdClientServer.h costantiServer.h
	$(CC) $(CFLAGS) server.c -o server

client: client.c cmdClientServer.h costantiClient.h
	$(CC) $(CFLAGS) client.c -o client

clean:
	rm *o client server