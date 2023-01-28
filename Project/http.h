#ifndef _HTTP_H_
#define _HTTP_H_

#include <unistd.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "config.h"

/*
 Request Design:
 GET /accounts\r\n
 Content-Length: 4\r\n
 Params: id=1&lang=vn\r\n
 \r\n
 body
 * */


/*
 Response Design:
 code: 200\r\n
 data: id=1&name=abc;id=2&name=xyz\r\n
 message: cookout
 * */

#include <stdio.h>

typedef struct Header {
  char command[CMD_L];
  char path[PATH_L];
  int content_l;
  char params[PARAM_L];
} Header;

typedef struct Body {
  char content[CONTENT_L];
} Body;

typedef struct Request {
  Header header;
  Body body;
} Request;

typedef struct Response {
  int code;
  char data[DATA_L];
  char message[MESSAGE_L];
  int send_type;
} Response;

typedef struct ClientAddr {
  int sock;
  struct sockaddr_storage __addr__;
  char address[ADDR_L];
} ClientAddr;

void cleanup(Request *, Response *, int *);
void req_print(Request);
void res_print(Response);

/* Parse request to string */
void req_parse(Request *, char *);

/* Parse string to response object */
void res_parse(Response *, char *);
void requestify(Request *, char *, char *, int, char *, char *);
void responsify(Response *, int, char *, char *, int);

void print_socket_addr(const struct sockaddr *, FILE *);
char *socket_addr(const struct sockaddr *);
int server_init(char *);
int connect2server(char *, char *);
ClientAddr accept_conn(int);
int get_req(int, Request *);
int get_res(int, Response *);
int send_req(int, Request);
int send_res(int *, Response);
void h_clear(char *, char *, char *);

#endif
