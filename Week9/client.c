#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include "config.h"
#include "http.h"

int clnt_sock;

void exit_safely() {
  z_send_req(clnt_sock, "EXIT HTTP/1.1\r\nContent-Length: 0\r\n\r\n");
  close(clnt_sock);
  printf("Bye :)\n");
  exit(SUCCESS);
}

void signalHandler(int signo) {
  switch (signo) {
    case SIGINT:
      z_warn("Caught signal Ctrl + C, coming out...\n");
      break;
    case SIGQUIT:
      z_warn("Caught signal Ctrl + \\, coming out...\n");
      break;
    case SIGHUP:
      z_warn("The terminal with the program (or some other parent if relevant) dies, coming out...\n");
      break;
    case SIGTERM:
      z_warn("The termination request (sent by the kill program by default and other similar tools), coming out...\n");
      break;
    case SIGUSR1:
      z_warn("Killing the program, coming out...\n");
      break;
    default: break;
  }

  exit_safely();
}

bool z_str_is_empty(char *str) {
  int length = (int)strlen(str);
  for(int i = 0; i < length; i++) {
    if(str[i] != 32)
      return FAILURE;
  }
  return SUCCESS;
}

void z_requestify(char *req, char *input) {
  char cmd[CMD_L], content[CONTENT_L];
  sscanf(input, "%s %[^\n]s", cmd, content);
  int content_l = (int)strlen(content);
  sprintf(req, "%s\r\nContent-Length: %d\r\n\r\n%s", cmd, content_l, content);
}

int main(int argc, char *argv[]) {
  if(argc != 3 || !z_is_ip(argv[1]) || !z_is_port(argv[2])) {
    z_error("\"Invalid parameter\nUsage: ./server <ipv4> <port>\n");
  }

  printf("\n\tMESSAGE PROGRAM\n");
  printf("\t=======================\n");

  clnt_sock = z_connect2server(argv[1], argv[2]);
  if(clnt_sock < 0) {
    z_error("Can't connect to server");
  }

  signal(SIGINT, signalHandler);
  signal(SIGQUIT, signalHandler);
  signal(SIGHUP, signalHandler);
  signal(SIGTERM, signalHandler);
  signal(SIGUSR1, signalHandler);

  int code;
  char input[CONTENT_L];
  char cmd[CMD_L], req[REQ_L], res[RES_L];

  do {
    z_clear(cmd, req, res);
    printf("[C]: ");
    scanf("%[^\n]s", input);
    z_clr_buff();
    z_requestify(req, input);
    z_send_req(clnt_sock, req);
    code = z_get_res(clnt_sock, res);
    printf("[S]: \x1b[1;38;5;47m%d\x1b[0m %s", code, res);
  } while(1);
}