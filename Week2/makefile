CFLAGS = -g -c -Wall
CC = gcc
LIBS = -lm

all: resolver

resolver: resolver.o utils.o error.o network.o
	${CC} resolver.o utils.o error.o network.o -o resolver $(info BUILD SUCCESS)

resolver.o: resolver.c
	${CC} ${CFLAGS} resolver.c

network.o: network.c
	${CC} ${CFLAGS} network.c

utils.o: utils.c
	${CC} ${CFLAGS} utils.c

error.o: error.c
	${CC} ${CFLAGS} error.c

clean:
	rm -f *.o *.~ | rm -rf resolver
