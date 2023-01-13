CFLAGS = -g -c -Wall
CC = gcc
LIBS = -lm

build: clean | client server

client: client.o http.o linkedlist.o
	${CC} client.o http.o linkedlist.o -o client

server: server.o serverHelper.o linkedlist.o http.o
	${CC} server.o serverHelper.o linkedlist.o http.o -o server

client.o: client.c
	${CC} ${CFLAGS} client.c

server.o: server.c
	${CC} ${CFLAGS} server.c

serverHelper.o: serverHelper.c
	${CC} ${CFLAGS} serverHelper.c

linkedlist.o: linkedlist.c
	${CC} ${CFLAGS} linkedlist.c

http.o: http.c
	${CC} ${CFLAGS} http.c

clean:
	rm -f *.o *~ | rm -rf client server