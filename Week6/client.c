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

void load_local_user(int sock) {
  char username[MAX_USERNAME] = "", alphas[MAX_PASSWORD] = "", numbers[MAX_PASSWORD] = "";
  FILE *fs = fopen("secret.txt", "r");
  int lineNo = 0;
  if(fs) {
    while(fscanf(fs, "%s %s %s", username, alphas, numbers)) {
      if(lineNo == 1) break;
      lineNo++;
    }

    if(strlen(username) != 0 && strlen(alphas) != 0) {
      char request[MAX_REQUEST_LENGTH], response[MAX_RESPONSE_LENGTH], method[MAX_HTTP_METHOD];
      strcpy(method, "GET");
      sprintf(request, "/accounts/remember/%s %s %s", username, alphas, numbers);
      send_request(sock, method, request);
      int code = get_response(sock, response);
      char greeting[100];
      if(code == 201) {
        sscanf(response, "201 success %s %s %[^\n]s", curr_user.username, curr_user.homepage, greeting);
        printf("\x1b[1;38;5;202m%s\x1b[0m]\n", greeting);
        logged_in = 1;
      }
    }
    fclose(fs);
  }
}

int main(int argc, char *argv[]) {
  // check port positive
  if(argc != 3 || !validate_ip(argv[1]) || !is_number(argv[2])) {
    err_error(ERR_INVALID_CLIENT_ARGUMENT);
    return FAIL;
  }

// loading();
  int sock = client_init_connect(argv[1], argv[2]);
  if(sock < 0) {
    err_error(ERR_CLIENT_CONNECT_FAILED);
    exit(FAIL);
  }

//  load_local_user(sock);

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