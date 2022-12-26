#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <sys/socket.h>

typedef struct Client {
  int sock;
  struct sockaddr_storage addr;
} Client;

bool compare_sockaddr(const struct sockaddr *, const struct sockaddr *);
char *get_socketaddr(const struct sockaddr *);
void print_socket_addr(const struct sockaddr *, FILE *);
void requestify(char *, char *);
int server_init_connect(char *);
int client_init_connect(char *, char *);
Client accept_connection(int);
int get_request(Client, char *, char *);
int get_response(int, char *);
int send_request(int, char *, char *);
int send_response(int, char *);
void http_clear(char *, char *, char *);