#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

typedef enum {
    _200_,
    _201_,
    _202_,
    _204_,
    _400_,
    _401_,
    _403_,
    _404_,
    _500_,
} HttpCode;

//extern int server_fd, client_fd;
//extern struct addrinfo *server, *client;

int http_code(HttpCode);
char *http_message(HttpCode);


bool compare_sockaddr(const struct sockaddr *, const struct sockaddr *);
char *get_socketaddr(const struct sockaddr *);
void requestify(char *, char *);
int server_init_connect(char *);
int client_init_connect(char *, char *);
int accept_connection(int);
int get_request(int, char *, char *);
int get_response(int, char *);
int send_request(int, char *, char *);
int send_response(int, char *);
void http_clear(char *, char *, char *);