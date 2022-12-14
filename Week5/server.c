#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "account.h"
#include "constants.h"
#include "error.h"
#include "http.h"
#include "linkedlist.h"
#include "network.h"
#include "utils.h"

XOR_LL acc_ll = XOR_LL_INITIALISER;
char gmethod[MAX_HTTP_METHOD], grequest[MAX_REQUEST_LENGTH], gresponse[MAX_RESPONSE_LENGTH];
int servSock, clntSock;

Account *findAccount(char *username) {
  XOR_LL_ITERATOR itr = XOR_LL_ITERATOR_INITIALISER;
  XOR_LL_LOOP_HTT_RST(&acc_ll, &itr) {
    Account *acc = (Account*)itr.node_data.ptr;
    if(strcmp(acc->username, username) == 0) return acc;
  }
  return NULL;
}

void encrypt(char *password, int key) {
  unsigned int i;
  for(i = 0; i < strlen(password); ++i) {
    password[i] = password[i] - key;
  }
}

void decrypt(char *password, int key) {
  unsigned int i;
  for(i = 0; i < strlen(password); ++i) {
    password[i] = password[i] + key;
  }
}

bool isValidPassword(char *password) {
  int i, passLen = strlen(password);
  for(i = 0; i < passLen; i++)
    if(isalnum(password[i]) == 0)
      return FAIL;

  return strcmp(password, "bye") == 0 ? FAIL : SUCCESS;
}

void encryptPassword(char *password, char *token, char *alphas, char *numbers) {
  int ia = 0, in = 0, it, i, passLen = strlen(password);
  strcpy(token, password);
  it = passLen;
  for(i = 0; i < passLen; i++) {
    if(isdigit(password[i])) {
      numbers[in++] = password[i];
      it += sprintf(&token[it], " %d", i);
    }
    else
      alphas[ia++] = password[i];
  }
  token[it] = '\0';
  alphas[ia] = '\0';
  numbers[in] = '\0';

  encrypt(token, KEY);
  encrypt(alphas, KEY);
  encrypt(numbers, KEY);
}

int comparePassword(char *password, char *password_input, char *alphas, char *numbers) {
  char password_tmp[MAX_PASSWORD];
  strcpy(password_tmp, password);
  decrypt(password_tmp, KEY);
  decrypt(alphas, KEY);
  if(!strlen(alphas) && !strlen(numbers)) {
    sscanf(password_tmp, "%s", password_tmp);
    return strcmp(password_tmp, password_input) == 0 ? SUCCESS : FAIL;
  }
  else if(!strlen(numbers)) {
    return strcmp(password_tmp, alphas) == 0 ? SUCCESS : FAIL;
  }
  else {
    char position[BUFFER] = "", pwd[MAX_PASSWORD];
    sscanf(password_tmp, "%s %[^\n]s", pwd, position);
    decrypt(numbers, KEY);

    char *token;
    int i, ia = 0, in = 0, passLen = strlen(pwd);
    char passwordEncrypted[MAX_PASSWORD];

    token = strtok(position, " ");
    int pos = (int)strtol(token, NULL, 10);

    for(i = 0; i < passLen; i++) {
      if(i == pos) {
        passwordEncrypted[i] = numbers[in++];
        token = strtok(position, " ");
        pos = token && (int)strtol(token, NULL, 10);
        continue;
      }
      passwordEncrypted[i] = alphas[ia++];
    }

    passwordEncrypted[i] = '\0';
    return strcmp(pwd, passwordEncrypted) ? SUCCESS : FAIL;
  }
}

int verifyUsername(char *request, char *response) {
  char username[MAX_USERNAME];
  sscanf(request, "/accounts/verify/username/%s", username);

  Account *acc = findAccount(username);
  if(!acc) {
    sprintf(response, "%s", "404 fail Account not exist");
    return FAIL;
  }

  sprintf(response, "200 success %s %d %d", acc->username, acc->num_time_wrong_code, acc->num_time_wrong_password);
  return SUCCESS;
}

int verifyPassword(char *request, char *response) {
  char username[MAX_USERNAME], password[MAX_PASSWORD];
  sscanf(request, "/accounts/verify/password/%s %s", username, password);

  if(isValidPassword(password)) {
    Account *acc = findAccount(username);
    if(!acc) {
      sprintf(response, "%s", "404 fail Account not exist");
      return FAIL;
    }

    if(!comparePassword(acc->password, password, "", "")) {
      strcpy(response, "400 fail Password incorrect");
      return FAIL;
    }

    sprintf(response, "%s", "200 success Password correct");
    return SUCCESS;
  }

  strcpy(response, "400 fail Password incorrect");
  return FAIL;
}

int rememberAccount(char *request, char *response) {
  char username[MAX_USERNAME] = "", alphas[MAX_PASSWORD] = "", numbers[MAX_PASSWORD] = "";
  sscanf(request, "/accounts/remember/%s %s %s", username, alphas, numbers);

  Account *acc = findAccount(username);
  if(!acc || !comparePassword(acc->password, "", alphas, numbers)) return FAIL;

  acc->status = -1;
  save_data(acc_ll);
  sprintf(response, "201 success %s %s Hello %s, have a nice day !", acc->username, acc->homepage, acc->username);
  return SUCCESS;
}

int login(char *request, char *response) {
  char username[MAX_USERNAME], password[MAX_PASSWORD];
  sscanf(request, "/accounts/authen?data: %s %s", username, password);

  Account *acc = findAccount(username);

  if(!comparePassword(acc->password, password, "", "")) {
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
  char token[MAX_PASSWORD], alphas[MAX_PASSWORD], numbers[MAX_PASSWORD];
  encryptPassword(password, token, alphas, numbers);
  strcpy(acc->password, token);
  save_data(acc_ll);

  sprintf(response, "202 success %s %s %s %s", acc->username, alphas, numbers, acc->homepage);
  return SUCCESS;
}

int createAccount(char *request, char *response) {
  Account *new_acc = (Account *)malloc(sizeof(*new_acc));
  sscanf(request, "/accounts/register?data: %s %s %s", new_acc->username, new_acc->password, new_acc->homepage);

  if(!isValidPassword(new_acc->password)) {
    sprintf(response, "%s", "400 fail Password incorrect");
    return FAIL;
  }

  char token[MAX_PASSWORD], alphas[MAX_PASSWORD], numbers[MAX_PASSWORD];
  encryptPassword(new_acc->password, token, alphas, numbers);
  strcpy(new_acc->password, token);

  // Default status = idle
  new_acc->status = 2;
  new_acc->num_time_wrong_password = new_acc->num_time_wrong_code = 0;

  xor_ll_push_tail(&acc_ll, new_acc, sizeof *new_acc);
  save_data(acc_ll);
  sprintf(response, "201 success %s %s %s %s", new_acc->username, alphas, numbers, new_acc->homepage);
  return SUCCESS;
}

int activateAccount(char *request, char *response) {
  char username[BUFFER], user_code[MAX_ACTIVATE_CODE_LENGTH];
  sscanf(request, "/accounts/activate?data: %s %s", username, user_code);

  Account *acc = findAccount(username);

  if(strcmp(user_code, ACTIVATION_CODE) != 0) {
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

int getAccount(char *request, char *response) {
  char username[MAX_USERNAME];
  sscanf(request, "/accounts/search/%s", username);

  Account *acc = findAccount(username);
  if(!acc) {
    sprintf(response, "%s", "404 fail Account not exist");
    return FAIL;
  }

  sprintf(response, "200 success %s %d %s", acc->username, acc->status, acc->homepage);
  return SUCCESS;
}

int updatePassword(char *request, char *response) {
  char username[MAX_USERNAME], new_password[MAX_PASSWORD];
  sscanf(request, "/accounts/updatePassword?data: %s %s", username, new_password);

  if(!isValidPassword(new_password)) {
    sprintf(response, "%s", "400 fail Password only have alpha, number and different string 'bye'");
    return FAIL;
  }

  Account *acc = findAccount(username);

  strcpy(acc->password, new_password);
  char token[MAX_PASSWORD], alphas[MAX_PASSWORD], numbers[MAX_PASSWORD];
  encryptPassword(acc->password, token, alphas, numbers);
  strcpy(acc->password, token);
  save_data(acc_ll);
  sprintf(response, "200 success %s %s %s %s", acc->username, alphas, numbers, acc->homepage);
  return SUCCESS;
}

int logout(char *request, char *response) {
  char username[MAX_USERNAME];
  sscanf(request, "/accounts/logout?data: %s", username);

  Account *acc = findAccount(username);
  if(acc || strcmp(username, "bye") == 0) {
    acc->status = 1;
    save_data(acc_ll);
    sprintf(response, "%s",  "202 success Logout successfully");
    return SUCCESS;
  }
  sprintf(response, "%s",  "400 fail Logout failed");
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
  clntSock = accept_connection(servSock);

  for(;;) {
    // Clear method, request, response
    http_clear(gmethod, grequest, gresponse);

    // HandleTCPClient(clntSock); -> Process client
    if(get_request(clntSock, gmethod, grequest) == FAIL) break;

    if(strcmp(gmethod, "GET") == 0) {
      route("/accounts/remember/", rememberAccount) ||
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

    send_response(clntSock, gresponse);
  }

  close(servSock);
  close(clntSock);
}

int main(int argc, char *argv[]) {
  if(argc != 2 || !is_number(argv[1])) {
    err_error(ERR_INVALID_SERVER_ARGUMENT);
    return FAIL;
  }

  // Set up connect
  servSock = server_init_connect(argv[1]);

  // Connect database
  xor_ll_init(&acc_ll);
  load_data(&acc_ll);

  // Listening request
  server_listen();

  printf("Mission successfully !\n");
  return SUCCESS;
}
