#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "account.h"
#include "constants.h"
#include "error.h"
#include "http.h"
#include "linkedlist.h"
#include "log.h"
#include "network.h"
#include "search.h"
#include "utils.h"

int main(int argc, char *argv[]) {
  // check port positive
  if(argc != 3 || !validate_ip(argv[1]) || !is_number(argv[2])) {
    err_error(ERR_INVALID_CLIENT_ARGUMENT);
    return FAIL;
  }

  client_init_connect(argv[1], argv[2]);
  loading();

  XOR_LL acc_ll = XOR_LL_INITIALISER;
  xor_ll_init(&acc_ll);
  load_data(&acc_ll);

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
      // Exit program
      xor_ll_destroy(&acc_ll);
      log_info("Thank for using my app :)\n");
      exit(EXIT_SUCCESS);
    }

    opt = input[0] - 48;
    switch(opt) {
      case 1:
        signup(&acc_ll);
        break;

      case 2:
        activate(&acc_ll);
        break;

      case 3:
        signin();
        break;

      case 4:
        search(acc_ll);
        break;

      case 5:
        change_password(&acc_ll);
        break;

      case 6:
        signout(&acc_ll);
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
  return SUCCESS;
}