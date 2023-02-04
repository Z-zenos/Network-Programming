#ifndef _HTTP_H_
#define _HTTP_H_

#include <unistd.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdbool.h>
#include <netdb.h>

#include "map.h"
#include "config.h"
#include "rbtree.h"

/*
 Message Design:
 PLAY#4#id=1&lang=vn#
 Hello anh em
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
void msg_print(Message);
int msg_parse(Message *, char *);
void server_error(Message *);
bool is_port(char *);
bool is_ip(const char *);
void messagify(Message *, char *, char *, char *, char *);
void responsify(Message *, char *, char *);
void parse_params(Message *, char *);
char *socket_addr(const struct sockaddr *);
int server_init(char *);
ClientAddr accept_conn(int);
int get_msg(int, Message *);
int send_msg(int *, Message);

#endif
