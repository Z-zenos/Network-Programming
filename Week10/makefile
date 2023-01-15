CFLAGS = -g -c -Wall
CC = gcc
LIBS = -lm 

build: clean | client server

client: client.o account.o utils.o error.o network.o http.o linkedlist.o
	${CC} client.o account.o utils.o error.o network.o http.o linkedlist.o -lpcre2-8 -o client

server: server.o account.o serverHelper.o utils.o error.o linkedlist.o network.o http.o
	${CC} server.o account.o serverHelper.o utils.o error.o linkedlist.o network.o http.o -lpcre2-8 -o server

client.o: client.c
	${CC} ${CFLAGS} client.c

server.o: server.c
	${CC} ${CFLAGS} server.c

serverHelper.o: serverHelper.c
	${CC} ${CFLAGS} serverHelper.c

network.o: network.c
	${CC} ${CFLAGS} network.c

linkedlist.o: linkedlist.c
	${CC} ${CFLAGS} linkedlist.c

account.o: account.c
	${CC} ${CFLAGS} account.c

http.o: http.c
	${CC} ${CFLAGS} http.c

utils.o: utils.c
	${CC} ${CFLAGS} utils.c

error.o: error.c
	${CC} ${CFLAGS} error.c

clean:
	rm -f *.o *~ | rm -rf client server
