#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include "config.h"
#include "http.h"
#include "utils.h"


int clnt_sock;

void exit_safely() {
//  send_req(clnt_sock, "EXIT \r\nContent-Length: 0\r\nParams: \r\n\r\n");
  close(clnt_sock);
  printf("Bye :)\n");
  exit(SUCCESS);
}

void signalHandler(int signo) {
  switch (signo) {
    case SIGINT:
      logger(L_WARN, 1, "Caught signal Ctrl + C, coming out...\n");
      break;
    case SIGQUIT:
      logger(L_WARN, 1, "Caught signal Ctrl + \\, coming out...\n");
      break;
    case SIGHUP:
      logger(L_WARN, 1, "The terminal with the program (or some other parent if relevant) dies, coming out...\n");
      break;
    case SIGTERM:
      logger(L_WARN, 1, "The termination request (sent by the kill program by default and other similar tools), coming out...\n");
      break;
    case SIGUSR1:
      logger(L_WARN, 1, "Killing the program, coming out...\n");
      break;
    default: break;
  }

  exit_safely();
}

bool str_is_empty(char *str) {
  int length = (int)strlen(str);
  for(int i = 0; i < length; i++) {
    if(str[i] != 32)
      return FAILURE;
  }
  return SUCCESS;
}



void make_req(Request *req, char *input) {
  sscanf(input, "%s %s %s %[^\n]s", req->header.command, req->header.path, req->header.params, req->body.content);
}

int main(int argc, char *argv[]) {
  printf("\n\tTEST API\n");
  printf("\t=======================\n");

  clnt_sock = connect2server(argv[1], argv[2]);
  if(clnt_sock < 0) {
    exit_safely();
  }

  signal(SIGINT, signalHandler);
  signal(SIGQUIT, signalHandler);
  signal(SIGHUP, signalHandler);
  signal(SIGTERM, signalHandler);
  signal(SIGUSR1, signalHandler);

  char input[CONTENT_L];
  char reqStr[REQ_L], resStr[RES_L], msg[RES_L], cmd[CMD_L];
  Request reqObj;
  Response resObj;

  do {
    h_clear(cmd, reqStr, resStr);
    strcpy(msg, "");
    printf("[C]: ");
    scanf("%[^\n]s", input);
    clear_buffer();

    strcpy(input, str_trim(input));
    if(strcmp(input, "EXIT") == 0) {
      exit_safely();
    }

    make_req(&reqObj, input);
    send_req(clnt_sock, reqObj);
    get_res(clnt_sock, &resObj);
    res_parse(&resObj, resStr);
    printf("[S]: \x1b[1;38;5;47m%d\x1b[0m \x1b[1;38;5;226m%s\x1b[0m\n", resObj.code, resObj.message);
  } while(1);
}