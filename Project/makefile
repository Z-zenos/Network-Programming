CFLAGS = -g -c -Wall -pthread
CC = gcc
LIBS = -lm

build: clean | client server

client: client.o http.o utils.o notify.o
	${CC} client.o http.o utils.o notify.o -o client

server: server.o auth.o http.o utils.o notify.o game.o algo.o player.o rbtree.o
	${CC} server.o auth.o http.o utils.o notify.o game.o algo.o player.o rbtree.o -L/usr/local/mysql/lib -lmysqlclient -lssl -lcrypto -pthread -o server

client.o: client.c
	${CC} ${CFLAGS} client.c

server.o: server.c
	${CC} ${CFLAGS} server.c

rbtree.o: rbtree.c
	${CC} ${CFLAGS} rbtree.c

http.o: http.c
	${CC} ${CFLAGS} http.c

auth.o: auth.c
	${CC} ${CFLAGS} auth.c

notify.o: notify.c
	${CC} ${CFLAGS} notify.c

utils.o: utils.c
	${CC} ${CFLAGS} utils.c

game.o: game.c
	${CC} ${CFLAGS} game.c

algo.o: algo.c
	${CC} ${CFLAGS} algo.c

player.o: player.c
	${CC} ${CFLAGS} player.c

clean:
	rm -f *.o *~ | rm -rf server client
