#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "account.h"
#include "constants.h"
#include "error.h"
#include "http.h"
#include "log.h"
#include "network.h"
#include "utils.h"

int sock;

int main(int argc, char *argv[]) {
  // check port positive
  if(argc != 3 || !validate_ip(argv[1]) || !is_number(argv[2])) {
    err_error(ERR_INVALID_CLIENT_ARGUMENT);
    return FAIL;
  }

  loading();
  client_init_connect(argv[1], argv[2]);

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
      log_info("Thank for using my app :)\n");
      close(sock);
      exit(EXIT_SUCCESS);
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
        signup();
        break;

      case 2:
        activate();
        break;

      case 3:
        signin();
        break;

      case 4:
        search();
        break;

      case 5:
        change_password();
        break;

      case 6:
        signout();
        break;

      case 7:
        get_domain();
        break;

      case 8:
        get_ipv4();
        break;

      default:
        break;
    }
  } while(1);
}