#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ctype.h>

#include "map.h"
#include "config.h"
#include "http.h"
#include "utils.h"

void cleanup(Message *msg, int *receiver) {
  memset(msg->command, '\0', CMD_L);
  memset(msg->content, '\0', CONTENT_L);
  memset(msg->__params__, '\0', PARAM_L);
  msg->content_l = 0;
  if(msg->params) map_drop(msg->params);
  msg->params = map_new();
  memset(receiver, 0, MAX_CLIENT);
}

void parse_params(Message *msg, char *params) {
  char **param_arr = str_split(params, '&');
  Object param;
  int i = 0;
  while (param_arr[i]) {
    if(sscanf(param_arr[i], "%[^=]=%s", param.key, param.value) == 1) memset(param.value, '\0', VAL_L);
    map_add(msg->params, param);
    i++;
  }
}

int msg_parse(Message *msg, char *msg_str) {
  char params[PARAM_L];
  if(sscanf(msg_str, "%[^#]#%d#%[^#]#%[^\n]", msg->command, &msg->content_l, msg->__params__, msg->content) < 3) return FAILURE;
  strcpy(params, msg->__params__);
  parse_params(msg, params);
  return SUCCESS;
}

void msg_print(Message msg) {
  printf("\n========\n");
  printf("Command: %s\n", msg.command);
  printf("Content-Length: %d\n", msg.content_l);
  printf("Params: %s\n", msg.__params__);
  printf("---------------\n");
  printf("%s\n", msg.content);
  printf("\n========\n");
}

void messagify(Message *msg, char *cmd, char *params, char *state, char *data) {
  strcpy(msg->command, cmd ? cmd : "RESPONSE");
  strcpy(msg->__params__, params ? params : "0");
  sprintf(msg->content, "state=%s", state);
  if(data) {
    strcat(msg->content, ",");
    strcat(msg->content, data);
  }
  msg->content_l = strlen(msg->content);
}

void responsify(Message *msg, char *state, char *data) {
  strcpy(msg->command, "RESPONSE");
  strcpy(msg->__params__, "0");
  sprintf(msg->content, "state=%s", state);
  if(data) {
    strcat(msg->content, ",");
    strcat(msg->content, data);
  }
  msg->content_l = strlen(msg->content);
}

void server_error(Message *msg) {
  responsify(msg, "server_error", NULL);
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
      break;
    }

    close(server_fd);
    server_fd = -1;
    exit(FAILURE);
  }

  freeaddrinfo(server);
  return server_fd;
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

int get_msg(int client_fd, Message *msg) {
  char msg_str[MSG_L];
  ssize_t numBytesRcvd = recv(client_fd, msg_str, MSG_L, 0);
  msg_str[numBytesRcvd - 1] = '\0';
  if(numBytesRcvd <= 0 || strlen(msg_str) <= 0) {
    return 0;
  }
  logger("success", msg_str);
  if(!msg_parse(msg, msg_str)) return 0;
  return numBytesRcvd;
}

int send_msg(int *receiver, Message msg) {
  char msg_str[MSG_L];
  sprintf(msg_str, "%s#%d#%s#%s\n", msg.command, msg.content_l, msg.__params__, msg.content);
  size_t msg_l = strlen(msg_str);
  for(int i = 0; i < MAX_SPECTATOR + 2; i++)
    if(receiver[i] > 0) send(receiver[i], msg_str, msg_l, 0);
  return SUCCESS;
}
