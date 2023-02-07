_mkfile_path := $(abspath $(lastword $(MAKEFILE_LIST)))
I := $(patsubst %/,%,$(dir $(_mkfile_path)))

ifneq ($(words $(MAKECMDGOALS)),1)
.DEFAULT_GOAL = build
%:
	@$(MAKE) $@ --no-print-directory -rRf $(firstword $(MAKEFILE_LIST))
else
ifndef ECHO
T := $(shell $(MAKE) $(MAKECMDGOALS) --no-print-directory \
      -nrRf $(firstword $(MAKEFILE_LIST)) \
      ECHO="COUNTTHIS" | grep -c "COUNTTHIS")
N := x
C = $(words $N)$(eval N := x $N)
ECHO = python progress.py --stepno=$C --nsteps=$T
endif

.PHONY: build clean

CFLAGS = -g -Wall
CC = gcc
LIBS = -lm

build: clean server
	@$(ECHO) "Build server success"

server: server.o auth.o http.o utils.o game.o player.o rbtree.o chat.o map.o
	@${CC} $(CFLAGS) server.o auth.o http.o utils.o game.o player.o rbtree.o chat.o map.o -L/usr/local/mysql/lib -lmysqlclient -lssl -lcrypto -pthread -o server
	@$(ECHO) Linking $@
	@sleep 0.05

%.o: %.c
	@$(CC) $(CFLAGS) -c $^
	@$(ECHO) Compiling $@
	@sleep 0.05

clean:
	@rm -f *.o *~ server

endif