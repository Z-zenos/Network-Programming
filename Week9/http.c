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

void http_clear(char *cmd, char *req, char *res) {
  memset(cmd, 0, CMD_L);
  memset(req, 0, CONTENT_L);
  memset(res, 0, CONTENT_L);
}

void z_clr_buff() {
  int c;
  while ((c = getchar()) != '\n' && c != EOF);
}

bool z_is_ip(const char *ip) {
  struct sockaddr_in sa;
  char ip_tmp[CONTENT_L];
  strcpy(ip_tmp, ip);
  // Convert ip address in xxx.xxx.xxx.xxx format to binary format
  int valid = inet_pton(AF_INET, ip_tmp, &(sa.sin_addr));
  return valid != 0;
}

bool z_is_port(char *str) {
  while (*str) {
    if (isdigit(*str++) == 0) return false;
  }
  return true;
}

void z_print_socket_addr(const struct sockaddr *address, FILE *stream) {
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

char *z_socket_addr(const struct sockaddr *address) {
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

int z_setup_server(char *service) {
  struct addrinfo addrConfig;
  memset(&addrConfig, 0, sizeof(addrConfig)); 
  addrConfig.ai_family = AF_INET; 
  addrConfig.ai_flags = AI_PASSIVE; 
  addrConfig.ai_socktype = SOCK_STREAM; 
  addrConfig.ai_protocol = IPPROTO_TCP; 

  struct addrinfo *server;
  if (getaddrinfo(NULL, service, &addrConfig, &server) != 0) {
    z_error("<%s> getaddrinfo() fail", __func__);
  }

  int server_fd = -1;
  for(struct addrinfo *addr = server; addr != NULL; addr = addr->ai_next) {
    server_fd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
    if (server_fd < 0) continue;

    if ((bind(server_fd, addr->ai_addr, addr->ai_addrlen) == 0) && (listen(server_fd, BACKLOG) == 0)) {
      struct sockaddr_storage localAddr;
      socklen_t addrSize = sizeof(localAddr);
      if(getsockname(server_fd, (struct sockaddr *) &localAddr, &addrSize) < 0) {
        z_error("<%s> getsockname() fail", __func__);
      }
      fputs("Server listening at: ", stdout);
      z_print_socket_addr((struct sockaddr *) &localAddr, stdout);
      fputc('\n', stdout);
      break;
    }

    close(server_fd);
    server_fd = -1;
    z_error("<%s> Bind / Listen fail", __func__);
  }

  freeaddrinfo(server);
  return server_fd;
}

int z_connect2server(char *server, char *port) {
  struct addrinfo addrConfig; 
  memset(&addrConfig, 0, sizeof(addrConfig)); 
  addrConfig.ai_family = AF_INET; 
  addrConfig.ai_socktype = SOCK_STREAM; 
  addrConfig.ai_protocol = IPPROTO_TCP; 

  struct addrinfo *servAddr;
  if (getaddrinfo(server, port, &addrConfig, &servAddr) != 0) {
    z_error("<%s> getaddrinfo fail", __func__);
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

Client z_accept(int server_fd) {
  Client client;
  socklen_t clientAddrLen = sizeof(client.addr);

  client.sock = accept(server_fd, (struct sockaddr *) &client.addr, &clientAddrLen);
  if (client.sock < 0) {
    z_error("<%s> accept denied", __func__);
  }
  return client;
}

int z_get_req(int client_fd, char *cmd, char *req) {
  ssize_t numBytesRcvd = recv(client_fd, req, REQ_L, 0);
  return numBytesRcvd;
}

int z_get_res(int server_fd, char *res) {
  int code;
  sscanf(res, "%d %[^\n]s", &code, res);
  ssize_t numBytes = recv(server_fd, res, RES_L, 0);
  return code;
}

int z_send_res(int client_fd, char *res, int code, char *msg) {
  sprintf(res, "{ code: %d, message: '%s' }", code, msg);
  size_t res_l = strlen(res);
  ssize_t numBytesSent = send(client_fd, res, res_l, 0);
  return numBytesSent;
}

int z_send_req(int server_fd, char *req) {
  size_t req_l = strlen(req);
  ssize_t numBytes = send(server_fd, req, req_l, 0);
  return numBytes;
}
