CFLAGS = -g -c -Wall
CC = gcc
LIBS = -lm 

all: main

main: main.o account.o utils.o error.o linkedlist.o
	${CC} main.o account.o utils.o error.o linkedlist.o -o main

main.o: main.c
	${CC} ${CFLAGS} main.c

linkedlist.o: linkedlist.c
	${CC} ${CFLAGS} linkedlist.c

account.o: account.c
	${CC} ${CFLAGS} account.c

utils.o: utils.c
	${CC} ${CFLAGS} utils.c

error.o: error.c
	${CC} ${CFLAGS} error.c

clean:
	rm -f *.o *~
