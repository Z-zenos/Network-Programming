#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "account.h"
#include "log.h"
#include "linkedlist.h"
#include "search.h"
#include "utils.h"

int main(int argc, char *argv[]) {
  loading();

  XOR_LL acc_ll = XOR_LL_INITIALISER;
  xor_ll_init(&acc_ll);
  load_data(&acc_ll);

  char input[1000];
  int opt;
  do {
//    load_data(&acc_ll);
    printf("\nUSER MANAGEMENT PROGRAM\n");
    printf("=======================\n");
    printf("[1]. Register\n");
    printf("[2]. Activate\n");
    printf("[3]. Sign in\n");
    printf("[4]. Search\n");
    printf("[5]. Change password\n");
    printf("[6]. Signout\n");
    printf("Your choice (1-6, other to quit): ");
    scanf("%[^\n]s", input);
    clear_buffer();

    if(!(strlen(input) == 1 && (input[0] > 48 && input[0] < 55))) {
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
        signin(&acc_ll);
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

      default:
        break;
    }
  } while(1);
}
