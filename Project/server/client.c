#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "config.h"
#include "http.h"
#include "utils.h"


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

int clnt_sock;

void exit_safely() {
  close(clnt_sock);
  printf("Bye :)\n");
  exit(SUCCESS);
}

void signalHandler(int signo) {
  switch (signo) {
    case SIGINT:
      logger(L_WARN, "Caught signal Ctrl + C, coming out...\n");
      break;
    case SIGQUIT:
      logger(L_WARN, "Caught signal Ctrl + \\, coming out...\n");
      break;
    case SIGHUP:
      logger(L_WARN, "The terminal with the program (or some other parent if relevant) dies, coming out...\n");
      break;
    case SIGTERM:
      logger(L_WARN, "The termination request (sent by the kill program by default and other similar tools), coming out...\n");
      break;
    case SIGUSR1:
      logger(L_WARN, "Killing the program, coming out...\n");
      break;
    default: break;
  }

  exit_safely();
}

void interrupt() {
  signal(SIGINT, signalHandler);
  signal(SIGQUIT, signalHandler);
  signal(SIGHUP, signalHandler);
  signal(SIGTERM, signalHandler);
  signal(SIGUSR1, signalHandler);
}

_Noreturn void send_handler() {
  interrupt();
  char input[CONTENT_L];
  Message msg;
  int receiver[MAX_SPECTATOR + 2];
  do {
    cleanup(&msg, receiver);
    printf("command: ");
    scanf("%s", msg.command);
    clear_buffer();

    printf("params: ");
    scanf("%s", msg.__params__);
    clear_buffer();

    printf("content: ");
    scanf("%s", msg.content);
    clear_buffer();

    msg.content_l = strlen(msg.content);

    strcpy(input, str_trim(input));
    if(strcmp(input, "EXIT") == 0) {
      exit_safely();
    }

    receiver[0] = clnt_sock;
    send_msg(receiver, msg);
  } while(1);
}

void recv_handler() {
  Message msg;
  int receiver[MAX_SPECTATOR + 2];
  int numBytesRcvd;
  while (1) {
    cleanup(&msg, receiver);
    numBytesRcvd = get_msg(clnt_sock, &msg);
    if(numBytesRcvd > 0)
      msg_print(msg);
    else break;
  }
}

int main(__attribute__((unused)) int argc, char *argv[]) {
  printf("\n\tTEST API\n");
  printf("\t=======================\n");

  clnt_sock = connect2server(argv[1], argv[2]);
  if(clnt_sock < 0) {
    exit_safely();
  }

  interrupt();

  pthread_t send_thread;
  if(pthread_create(&send_thread, NULL, (void *)send_handler, NULL) != 0) {
    logger(L_ERROR, "Create send thread failed");
    exit_safely();
  }

  pthread_t recv_thread;
  if(pthread_create(&recv_thread, NULL, (void *)recv_handler, NULL) != 0) {
    logger(L_ERROR, "Create receive thread failed");
    exit_safely();
  }
  while(1);
}
