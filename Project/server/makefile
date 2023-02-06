CFLAGS = -g -c -Wall -pthread
CC = gcc
LIBS = -lm

build: clean | server

server: server.o auth.o http.o utils.o game.o player.o rbtree.o chat.o map.o
	${CC} server.o auth.o http.o utils.o game.o player.o rbtree.o chat.o map.o -L/usr/local/mysql/lib -lmysqlclient -lssl -lcrypto -pthread -o server

server.o: server.c
	${CC} ${CFLAGS} server.c

rbtree.o: rbtree.c
	${CC} ${CFLAGS} rbtree.c

http.o: http.c
	${CC} ${CFLAGS} http.c

auth.o: auth.c
	${CC} ${CFLAGS} auth.c

map.o: map.c
	${CC} ${CFLAGS} map.c

utils.o: utils.c
	${CC} ${CFLAGS} utils.c

chat.o: chat.c
	${CC} ${CFLAGS} chat.c

game.o: game.c
	${CC} ${CFLAGS} game.c

player.o: player.c
	${CC} ${CFLAGS} player.c

clean:
	rm -f *.o *~ | rm -rf server
