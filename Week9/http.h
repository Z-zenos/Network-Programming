#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <sys/socket.h>

#include "config.h"

typedef struct Client {
  int sock;
  struct sockaddr_storage addr;
} Client;

typedef struct Header {
  char command[CMD_L];
  int content_length;
} Header;

typedef struct Body {
  char content[CONTENT_L];
} Body;

typedef struct Message {
  Header header;
  Body body;
} Message;

void z_print_socket_addr(const struct sockaddr *, FILE *);
char *z_socket_addr(const struct sockaddr *);
int z_setup_server(char *);
int z_connect2server(char *, char *);
Client z_accept(int);
int z_get_req(int, char *);
int z_get_res(int, char *);
int z_send_req(int, char *);
int z_send_res(int, char *, int, char*);
void z_clear(char *, char*);
bool z_is_ip(const char*);
bool z_is_port(char *);
void z_clr_buff();
void z_error(const char*, char*);
void z_warn(char *);