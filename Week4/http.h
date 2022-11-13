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

int http_code(HttpCode);
char *http_message(HttpCode);


bool compare_sockaddr(const struct sockaddr *, const struct sockaddr *);
void print_socketaddr(const struct sockaddr *, FILE *);
void requestify(char *, char *);
int server_init_connect(char *);
int client_init_connect(char *, char *);
int get_request(char *, char *);
int get_response(char *, char *);
int send_request(char *, char *, char *);
int send_response(char *);
void http_clear(char *, char *, char *);