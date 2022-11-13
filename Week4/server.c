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
char gmethod[MAX_HTTP_METHOD], grequest[MAX_REQUEST_LENGTH], gresponse[MAX_RESPONSE_LENGTH];

int verifyPassword(char *request, char *response) {
  char username[MAX_USERNAME], password[MAX_PASSWORD];
  sscanf(request, "/accounts/verify/password/%s %s", username, password);

  XOR_LL_ITERATOR itr = XOR_LL_ITERATOR_INITIALISER;
  XOR_LL_LOOP_HTT_RST(&acc_ll, &itr) {
    Account *acc = (Account*)itr.node_data.ptr;
    if(strcmp(acc->username, username) == 0 && strcmp(acc->password, password) == 0) {
      // responsify
      sprintf(response, "%s", "200 success Password correct");
      return SUCCESS;
    }
  }

  strcpy(response, "404 fail Password incorrect");
  return FAIL;
}

int verifyUsername(char *request, char *response) {
  char username[MAX_USERNAME];
  sscanf(request, "/accounts/verify/username/%s", username);

  XOR_LL_ITERATOR itr = XOR_LL_ITERATOR_INITIALISER;
  XOR_LL_LOOP_HTT_RST(&acc_ll, &itr) {
    Account *acc = (Account*)itr.node_data.ptr;
    if(strcmp(acc->username, username) == 0) {
      sprintf(response, "200 success %s %d %d", acc->username, acc->num_time_wrong_code, acc->num_time_wrong_password);
      return SUCCESS;
    }
  }

  strcpy(response, "404 fail Username not exist");
  return FAIL;
}

int getAccount(char *request, char *response) {
  char username[MAX_USERNAME];
  sscanf(request, "/accounts/search/%s", username);

  XOR_LL_ITERATOR itr = XOR_LL_ITERATOR_INITIALISER;
  XOR_LL_LOOP_HTT_RST(&acc_ll, &itr) {
    Account *acc = (Account*)itr.node_data.ptr;
    if(strcmp(acc->username, username) == 0) {
      sprintf(response, "200 success %s %d %s", acc->username, acc->status, acc->homepage);
      return SUCCESS;
    }
  }

  sprintf(response, "%s", "404 fail Account not exist");
  return FAIL;
}

int createAccount(char *request, char *response) {
  Account *new_acc = (Account *)malloc(sizeof(*new_acc));
  sscanf(request, "/accounts/register?data: %s %s %s", new_acc->username, new_acc->password, new_acc->homepage);

  // Default status = idle
  new_acc->status = 2;
  new_acc->num_time_wrong_password = new_acc->num_time_wrong_code = 0;

  xor_ll_push_tail(&acc_ll, new_acc, sizeof *new_acc);
  save_data(acc_ll);
  sprintf(response, "%s", "201 success Register successfully");
  return SUCCESS;
}

int updatePassword(char *request, char *response) {
  char username[MAX_USERNAME], new_password[MAX_PASSWORD];
  sscanf(request, "/accounts/updatePassword?data: %s %s", username, new_password);

  XOR_LL_ITERATOR itr = XOR_LL_ITERATOR_INITIALISER;
  XOR_LL_LOOP_HTT_RST(&acc_ll, &itr) {
    Account *acc = (Account*)itr.node_data.ptr;
    if(strcmp(acc->username, username) == 0) {
      strcpy(acc->password, new_password);
      save_data(acc_ll);
      sprintf(response, "%s", "200 success Update password successfully");
      return SUCCESS;
    }
  }

  sprintf(response, "%s", "400 fail Update password failed");
  return FAIL;
}

int activateAccount(char *request, char *response) {
  char username[BUFFER], user_code[MAX_ACTIVATE_CODE_LENGTH];
  sscanf(request, "/accounts/activate?data: %s %s", username, user_code);

  XOR_LL_ITERATOR itr = XOR_LL_ITERATOR_INITIALISER;
  XOR_LL_LOOP_HTT_RST(&acc_ll, &itr) {
    Account *acc = (Account*)itr.node_data.ptr;
    if(strcmp(acc->username, username) == 0 && strcmp(user_code, ACTIVATION_CODE) == 0) {
      if(acc->status == 1 || acc->status == -1) {
        sprintf(response, "%s", "204 fail Account activated");
        return FAIL;
      }
      acc->status = 1;
      acc->num_time_wrong_code = 0;
      acc->num_time_wrong_password = 0;
      save_data(acc_ll);
      sprintf(response, "%s", "200 success Activate account successfully");
      return SUCCESS;
    }
    else if(strcmp(acc->username, username) == 0) {
      ++acc->num_time_wrong_code;
      if(acc->num_time_wrong_code == MAX_WRONG_CODE) {
        acc->status = 0;
        sprintf(response, "%s", "403 fail Account blocked");
        save_data(acc_ll);
        return FAIL;
      }
      sprintf(response, "400 fail %d Activate code incorrect", acc->num_time_wrong_code);
      save_data(acc_ll);
      return FAIL;
    }
  }
  return FAIL;
}

int login(char *request, char *response) {
  char username[MAX_USERNAME];
  char password[MAX_PASSWORD];
  sscanf(request, "/accounts/authen?data: %s %s", username, password);

  XOR_LL_ITERATOR itr = XOR_LL_ITERATOR_INITIALISER;
  XOR_LL_LOOP_HTT_RST(&acc_ll, &itr) {
    Account *acc = (Account*)itr.node_data.ptr;
    if(strcmp(acc->username, username) == 0 && strcmp(acc->password, password) == 0) {
      // Check status of account(if account blocked/not activated -> return main menu)
      if(acc->status == 0) {
        sprintf(response, "%s", "401 fail Account blocked");
        return FAIL;
      }

      if(acc->status == 2) {
        sprintf(response, "%s", "401 fail Account non active");
        return FAIL;
      }

      acc->status = -1;
      acc->num_time_wrong_code = 0;
      acc->num_time_wrong_password = 0;
      save_data(acc_ll);
      sprintf(response, "202 success %s %s", acc->username, acc->homepage);
      return SUCCESS;
    }
    else if(strcmp(acc->username, username) == 0) {
      ++acc->num_time_wrong_password;
      if(acc->num_time_wrong_password == MAX_WRONG_PASSWORD) {
        acc->status = 0;
        save_data(acc_ll);
        sprintf(response, "403 fail %d Account blocked", acc->num_time_wrong_password);
        return FAIL;
      }
      save_data(acc_ll);
      sprintf(response, "400 fail %d %d Password incorrect", acc->num_time_wrong_code, acc->num_time_wrong_password);
      return FAIL;
    }
  }
  return FAIL;
}

int logout(char *request, char *response) {
  char username[MAX_USERNAME];
  sscanf(request, "/accounts/logout?data: %s", username);

  XOR_LL_ITERATOR itr = XOR_LL_ITERATOR_INITIALISER;
  XOR_LL_LOOP_HTT_RST(&acc_ll, &itr) {
    Account *acc = (Account*)itr.node_data.ptr;
    if(strcmp(acc->username, username) == 0) {
      acc->status = 1;
      save_data(acc_ll);
      sprintf(response, "%s",  "202 success Logout successfully");
      return SUCCESS;
    }
  }

  sprintf(response, "%s", "400 fail Logout failed");
  return FAIL;
}

int getIPv4(char *request, char *response) {
  char domain[BUFFER];
  sscanf(request, "/accounts/ipv4/%s", domain);

  char ipv4List[BUFFER];
  memset(ipv4List, 0, BUFFER);
  domain_name_to_ip(domain, ipv4List);
  if(strlen(ipv4List) == 0) {
    sprintf(response, "%s", "404 fail Can't find ipv4 address");
    return FAIL;
  }

  sprintf(response, "200 success %s", ipv4List);
  return SUCCESS;
}

int getDomain(char *request, char *response) {
  char ipv4[BUFFER];
  sscanf(request, "/accounts/domain/%s", ipv4);

  char domainList[BUFFER];
  memset(domainList, 0, BUFFER);
  ip_to_domain_name(ipv4, domainList);
  if(strlen(domainList) == 0) {
    sprintf(response, "%s", "404 fail Can't find domain name");
    return FAIL;
  }
  sprintf(response, "200 success %s", domainList);
  return SUCCESS;
}

int route(char *route_name, int (*f)(char *, char *)) {
  return str_start_with(grequest, route_name) && f(grequest, gresponse);
}

void server_listen() {
  for(;;) {
    // Clear method, request, response
    http_clear(gmethod, grequest, gresponse);

    get_request(gmethod, grequest);

    if(strcmp(gmethod, "GET") == 0) {
      route("/accounts/verify/username/", verifyUsername) ||
      route("/accounts/verify/password/", verifyPassword) ||
      route("/accounts/ipv4/", getIPv4) ||
      route("/accounts/domain/", getDomain) ||
      route("/accounts/search", getAccount) & 0;
    }
    else if(strcmp(gmethod, "POST") == 0) {
      route("/accounts/activate", activateAccount) ||
      route("/accounts/authen", login) ||
      route("/accounts/register", createAccount) & 0;
    }
    else if(strcmp(gmethod, "PATCH") == 0) {
      route("/accounts/updatePassword", updatePassword) ||
      route("/accounts/logout", logout) & 0;
    }

    send_response(gresponse);
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
