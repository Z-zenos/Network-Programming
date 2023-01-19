#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ctype.h>

#include "config.h"
#include "http.h"


void m_parse(Message *msg, char *req) {
  sscanf(req, "%s %s\r\nContent-Length: %d\r\nParams: %s\r\n\r\n%s", msg->header.command, msg->header.path, &msg->header.content_l, msg->header.params, msg->body.content);
}

void m_print(Message msg) {
  printf("%s %s\n", msg.header.command, msg.header.path);
  printf("Content-Length: %d\n", msg.header.content_l);
  printf("Params: %s\n", msg.header.params);
  printf("---------------\n");
  printf("%s\n", msg.body.content);
}

void clear(char *cmd, char *req, char *res) {
  memset(cmd, 0, CMD_L);
  memset(req, 0, REQ_L);
  memset(res, 0, RES_L);
}

bool is_ip(const char *ip) {    /* Handle login */
  struct sockaddr_in sa;
  char ip_tmp[CONTENT_L];
  strcpy(ip_tmp, ip);
  // Convert ip address in xxx.xxx.xxx.xxx format to binary format
  int valid = inet_pton(AF_INET, ip_tmp, &(sa.sin_addr));
  return valid != 0;
}

bool is_port(char *str) {
  while (*str) {
    if (isdigit(*str++) == 0) return false;
  }
  return true;
}

bool is_usr(char *str) {
  while (*str) {
    if (isalnum(*str++) == 0) return false;
  }
  return true;
}

char *trim(char *str) {
  if( str == NULL ) { return NULL; }
  if( str[0] == '\0' ) { return str; }

  size_t len = 0;
  char *frontp = str;
  char *endp = NULL;

  len = strlen(str);
  endp = str + len;

  while(isspace((unsigned char) *frontp)) { ++frontp; }
  if( endp != frontp ) {
    while( isspace((unsigned char) *(--endp)) && endp != frontp ) {}
  }

  if( frontp != str && endp == frontp )
    *str = '\0';
  else if( str + len - 1 != endp )
    *(endp + 1) = '\0';

  endp = str;
  if( frontp != str ) {
    while( *frontp ) { *endp++ = *frontp++; }
    *endp = '\0';
  }

  return str;
}

void print_socket_addr(const struct sockaddr *address, FILE *stream) {
  if (address == NULL || stream == NULL)
    return;
  void *numericAddress;
  char addrBuffer[INET6_ADDRSTRLEN];
  in_port_t port;
  switch (address->sa_family) {
    case AF_INET:
      numericAddress = &((struct sockaddr_in *) address)->sin_addr;
      port = ntohs(((struct sockaddr_in *) address)->sin_port);
      break;
    case AF_INET6:
      numericAddress = &((struct sockaddr_in6 *) address)->sin6_addr;
      port = ntohs(((struct sockaddr_in6 *) address)->sin6_port);
      break;
    default:
      fputs("[unknown type]", stream);
      return;
  }
  if (inet_ntop(address->sa_family, numericAddress, addrBuffer, sizeof(addrBuffer)) == NULL)
    fputs("[invalid address]", stream);
  else {
    fprintf(stream, "%s", addrBuffer);
    if (port != 0)
      fprintf(stream, "-%u", port);
  }
}

char *socket_addr(const struct sockaddr *address) {
  if (address == NULL)
    return NULL;

  void *numericAddress;
  char addrBuffer[INET6_ADDRSTRLEN];
  in_port_t port;
  switch (address->sa_family) {
    case AF_INET:
      numericAddress = &((struct sockaddr_in *) address)->sin_addr;
      port = ntohs(((struct sockaddr_in *) address)->sin_port);
      break;
    case AF_INET6:
      numericAddress = &((struct sockaddr_in6 *) address)->sin6_addr;
      port = ntohs(((struct sockaddr_in6 *) address)->sin6_port);
      break;
    default:
      return "unknown";
  }

  if (inet_ntop(address->sa_family, numericAddress, addrBuffer, sizeof(addrBuffer)) == NULL)
    return "unknown";
  else {
    char *addr = (char*)calloc(100, sizeof(char));
    sprintf(addr, "%s:%u", addrBuffer, port);
    return addr;
  }
}

int server_init(char *service) {
  struct addrinfo addrConfig;
  memset(&addrConfig, 0, sizeof(addrConfig));
  addrConfig.ai_family = AF_INET;
  addrConfig.ai_flags = AI_PASSIVE;
  addrConfig.ai_socktype = SOCK_STREAM;
  addrConfig.ai_protocol = IPPROTO_TCP;

  struct addrinfo *server;
  if (getaddrinfo(NULL, service, &addrConfig, &server) != 0) {
    exit(FAILURE);
  }

  int server_fd = -1;
  for(struct addrinfo *addr = server; addr != NULL; addr = addr->ai_next) {
    server_fd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
    if (server_fd < 0) continue;

    if ((bind(server_fd, addr->ai_addr, addr->ai_addrlen) == 0) && (listen(server_fd, BACKLOG) == 0)) {
      struct sockaddr_storage localAddr;
      socklen_t addrSize = sizeof(localAddr);
      if(getsockname(server_fd, (struct sockaddr *) &localAddr, &addrSize) < 0) {
        exit(FAILURE);
      }
      fputs("Server listening at: ", stdout);
      print_socket_addr((struct sockaddr *) &localAddr, stdout);
      fputc('\n', stdout);
      break;
    }

    close(server_fd);
    server_fd = -1;
    exit(FAILURE);
  }

  freeaddrinfo(server);
  return server_fd;
}

int connect2server(char *server, char *port) {
  struct addrinfo addrConfig;
  memset(&addrConfig, 0, sizeof(addrConfig));
  addrConfig.ai_family = AF_INET;
  addrConfig.ai_socktype = SOCK_STREAM;
  addrConfig.ai_protocol = IPPROTO_TCP;

  struct addrinfo *servAddr;
  if (getaddrinfo(server, port, &addrConfig, &servAddr) != 0) {
    exit(FAILURE);
  }

  int client_fd = -1;
  for(struct addrinfo *addr = servAddr; addr != NULL; addr = addr->ai_next) {
    client_fd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
    if (client_fd < 0) continue;

    if (connect(client_fd, addr->ai_addr, addr->ai_addrlen) == 0)
      break;

    close(client_fd);
    client_fd = -1;
  }

  freeaddrinfo(servAddr);
  return client_fd;
}

Client accept_conn(int server_fd) {
  Client client;
  socklen_t clientAddrLen = sizeof(client.addr);

  client.sock = accept(server_fd, (struct sockaddr *) &client.addr, &clientAddrLen);
  if (client.sock < 0) {
    exit(FAILURE);
  }
  return client;
}

int get_req(int client_fd, char *req) {
  ssize_t numBytesRcvd = recv(client_fd, req, REQ_L, 0);
  return numBytesRcvd;
}

int get_res(int server_fd, char *res) {
  recv(server_fd, res, RES_L, 0);
  return SUCCESS;
}

int send_res(int client_fd, char *res, int code, char *msg) {
  sprintf(res, "%d %s", code, msg);
  size_t res_l = strlen(res);
  ssize_t numBytesSent = send(client_fd, res, res_l, 0);
  return numBytesSent;
}

int send_req(int server_fd, char *req) {
  size_t req_l = strlen(req);
  ssize_t numBytes = send(server_fd, req, req_l, 0);
  return numBytes;
}