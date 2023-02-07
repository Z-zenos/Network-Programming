#ifndef _HTTP_H_
#define _HTTP_H_

#include <arpa/inet.h>
#include <netdb.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

#include "map.h"
#include "config.h"
#include "rbtree.h"

/*
 Message Design:
  + Message(Request)
      PLAY#4#id=1&lang=vn&game=1#
      this is body
  + Message(Response)
      RESPONSE#4#body_length&0#
      state=chat_global,may be chat something
 * */

typedef struct Message {
  /* Header */
  char command[CMD_L];
  int content_l;
  Map *params;
  char __params__[PARAM_L];

  /* Body */
  char content[CONTENT_L];
} Message;

typedef struct ClientAddr {
  int sock;
  struct sockaddr_storage __addr__;
  char address[ADDR_L];
} ClientAddr;

void cleanup(Message *, int *);
int msg_parse(Message *, char *);
void responsify(Message *, char *, char *);

void server_error(Message *);
void parse_params(Message *, char *);
char *socket_addr(const struct sockaddr *);
int server_init(char *);
ClientAddr accept_conn(int);
int get_msg(int, Message *);
int send_msg(int *, Message);

#endif
