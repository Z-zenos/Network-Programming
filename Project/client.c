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
  Request req = {
    .header = {
      .command = "EXIT",
      .path = "/exit",
      .params = "",
      .content_l = 0
    },
    .body.content = ""
  };

  send_req(clnt_sock, req);
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
  Request reqObj;
  Response resObj;
  int receiver[MAX_SPECTATOR + 2];
  do {
    cleanup(&reqObj, &resObj, receiver);
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
    printf("[S]: \n\t%d\n\t%s\n\t%s\n", resObj.code, resObj.data, resObj.message);
  } while(1);
}