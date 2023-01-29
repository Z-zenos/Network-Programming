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
#include "utils.h"

void cleanup(Request *req, Response *res, int *receiver) {
  res->send_type = SEND_ME;
  memset(req->header.command, '\0', CMD_L);
  memset(req->header.path, '\0', PATH_L);
  memset(req->header.params, '\0', PARAM_L);
  memset(req->body.content, '\0', CONTENT_L);
  memset(res->data, '\0', DATA_L);
  memset(res->message, '\0', MESSAGE_L);
  memset(receiver, 0, MAX_SPECTATOR + 2);
}

void req_parse(Request *req, char *str) {
  sscanf(str, "%s %s\r\nContent-Length: %d\r\nParams: %s\r\n\r\n%[^\n]", req->header.command, req->header.path, &req->header.content_l, req->header.params, req->body.content);
}

void res_parse(Response *res, char *str) {
  sscanf(str, "code: %d\r\ndata: %[^\n]\r\nmessage: %[^\n]", &res->code, res->data, res->message);
}

void req_print(Request req) {
  printf("\n========\n");
  printf("%s %s\n", req.header.command, req.header.path);
  printf("Content-Length: %d\n", req.header.content_l);
  printf("Params: %s\n", req.header.params);
  printf("---------------\n");
  printf("%s\n", req.body.content);
  printf("\n========\n");
}

void res_print(Response res) {
  printf("\n========\n");
  printf("code: %d\n", res.code);
  printf("data: %s\n", res.data);
  printf("message: %s\n", res.message);
  printf("\n========\n");
}

void responsify(Response *res, int code, char *data, char *msg, int send_type) {
  res->code = code;
  strcpy(res->data, data ? data : "null");
  strcpy(res->message, msg ? msg : "null");
  res->send_type = send_type;
}

void requestify(Request *req, char *cmd, char *path, int content_l, char *params, char *content) {
  strcpy(req->header.command, cmd ? cmd : "null");
  strcpy(req->header.path, path ? path : "null");
  req->header.content_l = content_l;
  strcpy(req->header.params, params ? params : "null");
  strcpy(req->body.content, content ? content : "null");
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
      logger(L_SUCCESS, "Server listening at: %s", socket_addr((struct sockaddr *) &localAddr));
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
    logger(L_ERROR, "Can't find server address...");
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

ClientAddr accept_conn(int server_fd) {
  ClientAddr client_addr;
  socklen_t clientAddrLen = sizeof(client_addr.__addr__);

  client_addr.sock = accept(server_fd, (struct sockaddr *) &client_addr.__addr__, &clientAddrLen);
  if (client_addr.sock < 0) {
    exit(FAILURE);
  }
  strcpy(client_addr.address, socket_addr((struct sockaddr *) &client_addr.__addr__));
  return client_addr;
}

int get_req(int client_fd, Request *req) {
  char reqStr[REQ_L];
  ssize_t numBytesRcvd = recv(client_fd, reqStr, REQ_L, 0);
  reqStr[numBytesRcvd] = '\0';
  if(numBytesRcvd <= 0) {

  }
  req_parse(req, reqStr);
  return numBytesRcvd;
}

int get_res(int server_fd, Response *res) {
  char resStr[RES_L];
  ssize_t numBytesRcvd = recv(server_fd, resStr, RES_L, 0);
  resStr[numBytesRcvd] = '\0';
  res_parse(res, resStr);
  return numBytesRcvd;
}

int send_res(int *receiver, Response res) {
  char resStr[RES_L];
  sprintf(resStr, "code: %d\r\ndata: %s\r\nmessage: %s", res.code, res.data, res.message);
  size_t res_l = strlen(resStr);
  for(int i = 0; i < MAX_SPECTATOR + 2; i++)
    if(receiver[i] > 0) send(receiver[i], resStr, res_l, 0);
  return SUCCESS;
}

int send_req(int server_fd, Request req) {
  char reqStr[REQ_L];
  sprintf(reqStr, "%s %s\r\nContent-Length: %d\r\nParams: %s\r\n\r\n%s", req.header.command, req.header.path, req.header.content_l, req.header.params, req.body.content);
  size_t req_l = strlen(reqStr);
  ssize_t numBytesSent = send(server_fd, reqStr, req_l, 0);
  return numBytesSent;
}
