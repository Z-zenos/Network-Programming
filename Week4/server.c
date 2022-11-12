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
#include "utils.h"

XOR_LL acc_ll = XOR_LL_INITIALISER;

int getAccount(char *request, char *response) {
  sscanf(request, "/accounts/%s", request);
  XOR_LL_ITERATOR itr = XOR_LL_ITERATOR_INITIALISER;

  XOR_LL_LOOP_HTT_RST(&acc_ll, &itr) {
    Account *acc = (Account*)itr.node_data.ptr;
    if(strcmp(acc->username, request) == 0) {
      // responsify
      sprintf(response, "%s %s %d %d %d %s", acc->username, acc->password, acc->status, acc->num_time_wrong_code, acc->num_time_wrong_password, acc->homepage);
      return SUCCESS;
    }
  }

  return FAIL;
}

int createAccount(char *request, char *response) {
  Account *new_account = (Account *)malloc(sizeof(*new_account));
  sscanf(
    request,
    "/accounts?data: %s %s %d %d %d %s",
    new_account->username, new_account->password, &new_account->status, &new_account->num_time_wrong_code, &new_account->num_time_wrong_password, new_account->homepage
  );

  xor_ll_push_tail(&acc_ll, new_account, sizeof *new_account);
  save_data(acc_ll);
  strcpy(response, "201 success Register successfully");
  return SUCCESS;
}

void server_listen() {
  char method[10], request[BUFFER], response[BUFFER];

  for(;;) {
    // Clear method, request, response
    strcpy(method, "");
    strcpy(request, "");
    strcpy(response, "");

    get_request(method, request, response);

    if(strcmp(method, "GET") == 0) {
      if(str_start_with(request, "/accounts/")) getAccount(request, response);

    }
    else if(strcmp(method, "POST") == 0) {
      if(str_start_with(request, "/accounts?data:")) createAccount(request, response);

    }
    else if(strcmp(method, "PATCH") == 0) {

    }

    send_response(method, request, response);
  }
}

int main(int argc, char *argv[]) {
  if(argc != 2 || !is_number(argv[1])) {
    err_error(ERR_INVALID_SERVER_ARGUMENT);
    return FAIL;
  }

  // Set up connect
  server_init_connect(argv[1]);

  // Connect database
  xor_ll_init(&acc_ll);
  load_data(&acc_ll);

  // Listening request
  server_listen();
  return SUCCESS;
}
