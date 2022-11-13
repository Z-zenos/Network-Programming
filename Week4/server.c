#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "account.h"
#include "constants.h"
#include "error.h"
#include "http.h"
#include "linkedlist.h"
#include "network.h"
#include "utils.h"

XOR_LL acc_ll = XOR_LL_INITIALISER;

int verifyAccount(char *request, char *response) {
  sscanf(request, "/verify/%s", request);
  XOR_LL_ITERATOR itr = XOR_LL_ITERATOR_INITIALISER;

  XOR_LL_LOOP_HTT_RST(&acc_ll, &itr) {
    Account *acc = (Account*)itr.node_data.ptr;
    if(strcmp(acc->username, request) == 0) {
      // responsify
      sprintf(response, "200 success %s %d %d", acc->username, acc->num_time_wrong_code, acc->num_time_wrong_password);
      return SUCCESS;
    }
  }

  strcpy(response, "404 fail");
  return FAIL;
}

int getAccount(char *request, char *response) {
  sscanf(request, "/accounts/%s", request);
  XOR_LL_ITERATOR itr = XOR_LL_ITERATOR_INITIALISER;

  XOR_LL_LOOP_HTT_RST(&acc_ll, &itr) {
    Account *acc = (Account*)itr.node_data.ptr;
    if(strcmp(acc->username, request) == 0) {
      // responsify
      sprintf(response, "200 success %s %s %d %d %d %s", acc->username, acc->password, acc->status, acc->num_time_wrong_code, acc->num_time_wrong_password, acc->homepage);
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

int updateAccount(char *request, char *response) {
  Account *update_account = (Account *)malloc(sizeof(*update_account));
  sscanf(
    request,
    "/accounts?data: %s %d %d %d",
    update_account->username, &update_account->status, &update_account->num_time_wrong_code, &update_account->num_time_wrong_password
  );

  XOR_LL_ITERATOR itr = XOR_LL_ITERATOR_INITIALISER;
  XOR_LL_LOOP_HTT_RST(&acc_ll, &itr) {
    Account *acc = (Account*)itr.node_data.ptr;
    if(strcmp(acc->username, update_account->username) == 0) {
      // responsify
      acc->status = update_account->status;
      acc->num_time_wrong_code = update_account->num_time_wrong_code;
      acc->num_time_wrong_password = update_account->num_time_wrong_password;

      save_data(acc_ll);
      sprintf(response, "200 success Login successfully");
      return SUCCESS;
    }
  }

  return FAIL;
}

int login(char *request, char *response) {
  char username[MAX_USERNAME];
  char password[MAX_PASSWORD];
  sscanf(request, "/authen?data: %s %s", username, password);
  XOR_LL_ITERATOR itr = XOR_LL_ITERATOR_INITIALISER;

  XOR_LL_LOOP_HTT_RST(&acc_ll, &itr) {
    Account *acc = (Account*)itr.node_data.ptr;
    if(strcmp(acc->username, username) == 0 && strcmp(acc->password, password) == 0) {
      // Check status of account(if account blocked/not activated -> return main menu)
      if(acc->status == 0) {
        sprintf(response, "%s", "403 fail Account blocked");
        return FAIL;
      }

      if(acc->status == 2) {
        sprintf(response, "%s", "403 fail Account non active");
        return FAIL;
      }

      acc->status = -1;
      acc->num_time_wrong_code = 0;
      acc->num_time_wrong_password = 0;
      save_data(acc_ll);
      sprintf(response, "202 success %s %s %d %d %d %s", acc->username, acc->password, acc->status, acc->num_time_wrong_code, acc->num_time_wrong_password, acc->homepage);
      return SUCCESS;
    }
    else if(strcmp(acc->username, username) == 0) {
      ++acc->num_time_wrong_password;
      if(acc->num_time_wrong_password == MAX_WRONG_PASSWORD) {
        acc->status = 0;
      }
      save_data(acc_ll);
      sprintf(response, "401 fail %d %d", acc->num_time_wrong_code, acc->num_time_wrong_password);
      return FAIL;
    }
  }
  return FAIL;
}

void server_listen() {
  char method[10], request[BUFFER], response[BUFFER];
  for(;;) {
    // Clear method, request, response
    http_clear(method, request, response);

    get_request(method, request);

    if(strcmp(method, "GET") == 0) {
      if(str_start_with(request, "/verify/")) verifyAccount(request, response);
      else if(str_start_with(request, "/accounts/")) getAccount(request, response);

    }
    else if(strcmp(method, "POST") == 0) {
      if(str_start_with(request, "/accounts?data:")) createAccount(request, response);
      if(str_start_with(request, "/authen?data:")) login(request, response);

    }
    else if(strcmp(method, "PATCH") == 0) {

    }

    send_response(response);
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
