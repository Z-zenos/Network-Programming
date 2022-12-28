CFLAGS = -g -c -Wall -pthread
CC = gcc
LIBS = -lm

build: clean | server

server: server.o auth.o utils.o message.o
	${CC} server.o auth.o utils.o message.o -L/usr/local/mysql/lib -lmysqlclient -o server

server.o: server.c
	${CC} ${CFLAGS} server.c

auth.o: auth.c
	${CC} ${CFLAGS} auth.c

message.o: message.c
	${CC} ${CFLAGS} message.c

utils.o: utils.c
	${CC} ${CFLAGS} utils.c

clean:
	rm -f *.o *~ | rm -rf server
