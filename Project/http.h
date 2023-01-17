
#ifndef _HTTP_H_
#define _HTTP_H_

#include <unistd.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "config.h"

/*
 Message Design Example:
 GET /accounts\r\n
 Content-Length: 4\r\n
 Params: id=1&lang=vn\r\n
 \r\n
 body
 * */

typedef struct Header {
  char command[CMD_L];
  char path[PATH_L];
  int content_l;
  char params[PARAM_L];
} Header;

typedef struct Body {
  char content[CONTENT_L];
} Body;

typedef struct Message {
  Header header;
  Body body;
} Message;

Message m_parse(char *);


void print_socket_addr(const struct sockaddr *, FILE *);
char *socket_addr(const struct sockaddr *);
int setup_server(char *);
int connect2server(char *, char *);
Client accept(int);
int get_req(int, char *);
int get_res(int, char *);
int send_req(int, char *);
int send_res(int, char *, int, char*);
void clear(char *, char*);

#endif
