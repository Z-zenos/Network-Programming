#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include "config.h"
#include "http.h"
#include "utils.h"
#include "log.h"

int clnt_sock;

void exit_safely() {
  send_req(clnt_sock, "EXIT HTTP/1.1\r\nContent-Length: 0\r\n\r\n");
  close(clnt_sock);
  printf("Bye :)\n");
  exit(SUCCESS);
}

void signalHandler(int signo) {
  switch (signo) {
    case SIGINT:
      log_warn("Caught signal Ctrl + C, coming out...\n");
      break;
    case SIGQUIT:
      log_warn("Caught signal Ctrl + \\, coming out...\n");
      break;
    case SIGHUP:
      log_warn("The terminal with the program (or some other parent if relevant) dies, coming out...\n");
      break;
    case SIGTERM:
      log_warn("The termination request (sent by the kill program by default and other similar tools), coming out...\n");
      break;
    case SIGUSR1:
      log_warn("Killing the program, coming out...\n");
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

void requestify(char *req, char *input) {
  char cmd[CMD_L], content[CONTENT_L], path[PATH_L], params[PARAM_L];
  sscanf(input, "%s %s %s %[^\n]s", cmd, path, params, content);
  strcpy(content, str_trim(content));
  int content_l = (int)strlen(content);
  sprintf(req, "%s %s\r\nContent-Length: %d\r\nParams: %s\r\n\r\n%s", cmd, path, content_l, params, content);
}

int main(int argc, char *argv[]) {


  printf("\n\tMESSAGE PROGRAM\n");
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

  int code;
  char input[CONTENT_L];
  char req[REQ_L], res[RES_L], msg[RES_L], cmd[CMD_L];


  do {
    clear(cmd, req, res);
    strcpy(msg, "");
    printf("[C]: ");
    scanf("%[^\n]s", input);
    clear_buffer();

    strcpy(input, str_trim(input));
    if(strcmp(input, "EXIT") == 0) {
      exit_safely();
    }

    requestify(req, input);
    send_req(clnt_sock, req);
    get_res(clnt_sock, res);
    sscanf(res, "%d %[^\n]s", &code, msg);
    printf("[S]: \x1b[1;38;5;47m%d\x1b[0m \x1b[1;38;5;226m%s\x1b[0m\n", code, msg);
  } while(1);
}