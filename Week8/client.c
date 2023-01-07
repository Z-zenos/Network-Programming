#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include "account.h"
#include "constants.h"
#include "error.h"
#include "http.h"
#include "log.h"
#include "network.h"
#include "utils.h"

int sock;

void exit_safely() {
  send_request(sock, "", "");
  close(sock);
  log_info("Thank for using my app :)\n");
  exit(EXIT_SUCCESS);
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
  }

  exit_safely();
}


int main(int argc, char *argv[]) {
  if(argc != 3 || !validate_ip(argv[1]) || !is_number(argv[2])) {
    err_error(ERR_INVALID_CLIENT_ARGUMENT);
    return FAIL;
  }

  sock = client_init_connect(argv[1], argv[2]);
  if(sock < 0) {
    err_error(ERR_CLIENT_CONNECT_FAILED);
    exit(FAIL);
  }

  signal(SIGINT, signalHandler);
  signal(SIGQUIT, signalHandler);
  signal(SIGHUP, signalHandler);
  signal(SIGTERM, signalHandler);
  signal(SIGUSR1, signalHandler);

  char input[1000];
  int opt;
  do {
    printf("\n\tUSER MANAGEMENT PROGRAM\n");
    printf("\t=======================\n");
    printf("%-30s%s\n", "[1]. Register", "[5]. Change password");
    printf("%-30s%s\n", "[2]. Activate", "[6]. Sign out");
    printf("%-30s%s\n", "[3]. Sign in",  "[7]. Homepage with domain name");
    printf("%-30s%s\n", "[4]. Search",   "[8]. Homepage with IP address");
    printf("Your choice (1-8, other to quit): ");
    scanf("%[^\n]s", input);
    clear_buffer();

    if(!(strlen(input) == 1 && (input[0] > 48 && input[0] < 57))) {
      exit_safely();
    }

    opt = input[0] - 48;
    if(opt >=4 && opt <= 8) {
      if(!logged_in) {
        err_error(ERR_NON_LOG_IN);
        continue;
      }
    }
    switch(opt) {
      case 1:
        signup(sock);
        break;

      case 2:
        activate(sock);
        break;

      case 3:
        signin(sock);
        break;

      case 4:
        search(sock);
        break;

      case 5:
        change_password(sock);
        break;

      case 6:
        signout(sock);
        break;

      case 7:
        get_domain(sock);
        break;

      case 8:
        get_ipv4(sock);
        break;

      default:
        break;
    }
  } while(1);
}