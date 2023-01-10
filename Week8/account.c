#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "network.h"
#include "account.h"
#include "error.h"
#include "http.h"
#include "log.h"
#include "utils.h"

extern int logged_in;
extern Account curr_user;

char method[MAX_METHOD_LENGTH], request[MAX_REQUEST_LENGTH], response[MAX_RESPONSE_LENGTH];

// Set current user
void _set_current_user_(Account acc) {
  strcpy(curr_user.username, acc.username);
  memset(curr_user.password, 0, MAX_PASSWORD);
  strcpy(curr_user.homepage, acc.homepage);
}

// Logout user
void _reset_current_user_() {
  memset(curr_user.username, 0, MAX_USERNAME);
  memset(curr_user.password, 0, MAX_PASSWORD);
  memset(curr_user.homepage, 0, MAX_HOMEPAGE);
}

void secretify(char *str) {
  char uname[MAX_USERNAME], alphas[MAX_PASSWORD], numbers[MAX_PASSWORD];
  int code;
  sscanf(str, "%d success %s %s %s %s", &code, uname, alphas, numbers, curr_user.homepage);

  FILE *fs = fopen("secret_tmp.txt", "w");
  rewind(fs);
  fprintf(fs, "%s %s %s", uname, alphas, numbers);
  remove("secret.txt");
  rename("secret_tmp.txt", "secret.txt");
  fclose(fs);
}

int verify_username(int sock, char *username, int *num_time_wrong_code, int *num_time_wrong_password) {
  http_clear(method, request, response);
  strcpy(method, "GET");
  sprintf(request, "/accounts/verify/username/%s", username);

  send_request(sock, method, request);
  int code = get_response(sock, response);

  if(!num_time_wrong_code && !num_time_wrong_password)
    sscanf(response, "200 success %s", username);
  else sscanf(response, "200 success %s %d %d", username, num_time_wrong_code, num_time_wrong_password);

  if(code == 200) return SUCCESS;
  if(code == 404) return FAIL;
  return FAIL;
}

int verify_password(int sock, char *username, char *password) {
  http_clear(method, request, response);
  strcpy(method, "GET");
  sprintf(request, "/accounts/verify/password/%s %s", username, password);

  send_request(sock, method, request);
  int code = get_response(sock, response);

  if(code == 200) return SUCCESS;
  if(code == 404) return FAIL;
  return FAIL;
}

Account *search_account(int sock, char *username) {
  http_clear(method, request, response);
  strcpy(method, "GET");
  sprintf(request, "/accounts/search/%s", username);

  send_request(sock, method, request);
  int code = get_response(sock, response);
  if (code == 200) {
    Account *acc = (Account *) malloc(sizeof(*acc));
    sscanf(response, "200 success %s %d %s", acc->username, &acc->status, acc->homepage);
    return acc;
  }
  else return NULL;
}

void signup(int sock) {
  printf("\n===== Register =====\n");

  char username_input[MAX_USERNAME];
  input("Username", username_input, MAX_USERNAME, false);

  // Check account if exist
  if(verify_username(sock, username_input, NULL, NULL)) {
    err_error(ERR_ACCOUNT_EXISTED);
    return;
  }
  else {
    // Create new account
    Account *new_account = (Account *)malloc(sizeof(*new_account));

    strcpy(new_account->username, username_input);
    input("Password", new_account->password, MAX_PASSWORD, true);
    input("Homepage", new_account->homepage, MAX_HOMEPAGE, false);

    if(!validate_domain_name(new_account->homepage) && !validate_ip(new_account->homepage)) {
      err_error(ERR_INVALID_HOMEPAGE_ADDRESS);
      return;
    }

    http_clear(method, request, response);
    strcpy(method, "POST");
    sprintf(request, "/accounts/register?data: %s %s %s", new_account->username, new_account->password, new_account->homepage);
    send_request(sock, method, request);
    int code = get_response(sock, response);
    if(code != 201) {
      err_error(ERR_REGISTER_ACCOUNT_FAILED);
      return;
    }
    secretify(response);
    log_success("Register successfully");
  }
}

void activate(int sock) {
  printf("\n===== Activate Account =====\n");
  char username_input[MAX_USERNAME];
  input("Username", username_input, MAX_USERNAME, false);

  // Can't activate account yourself
  if(strcmp(curr_user.username, username_input) == 0) {
    log_warn("Can't activate account signing in.");
    return;
  }

  int num_time_wrong_code, num_time_wrong_password;
  if(!verify_username(sock, username_input, &num_time_wrong_code, &num_time_wrong_password)) {
    err_error(ERR_ACCOUNT_NOT_FOUND);
    return;
  }

  char password_input[MAX_PASSWORD];
  input("Password", password_input, MAX_PASSWORD, true);

  if(!verify_password(sock, username_input, password_input)) {
    err_error(ERR_PASSWORD_INCORRECT);
    return;
  }

  char activation_code[MAX_ACTIVATE_CODE_LENGTH];
  char opt[BUFFER];

  do {
    // Reset activation code
    strcpy(activation_code, "");
    strcpy(opt, "");

    if(num_time_wrong_code < MAX_WRONG_CODE && num_time_wrong_code > 0) {
      log_warn("You remain %d time(s) input activation code!", MAX_WRONG_CODE - num_time_wrong_code);
    }

    // Input activation code
    printf("Activation code: ");
    scanf("%[^\n]s", activation_code);
    clear_buffer();

    // Check if activation code is empty
    if(strlen(activation_code) == 0) {
      err_error(ERR_INPUT_EMPTY);
      continue;
    }

    http_clear(method, request, response);
    strcpy(method, "POST");
    sprintf(request, "/accounts/activate?data: %s %s", username_input, activation_code);
    send_request(sock, method, request);
    int code = get_response(sock, response);

    // Activate code correct but account has been activated.
    switch (code) {
      case 200:
        log_success("%s", response);
        return;

      case 204:
        log_warn("%s", response);
        return;

      case 400:
        sscanf(response, "400 fail %d %[^\n]s", &num_time_wrong_code, response);
        log_error("%s", response);
        break;

      case 403:
        log_error("%s", response);
        return;
    }

    do {
      input("Would you like to continue? (y/n)", opt, 1, false);
    } while(!(opt[0] == 'y' || opt[0] == 'n'));
    if(opt[0] == 'y') continue;
    else if(opt[0] == 'n') break;

  } while(num_time_wrong_code < MAX_WRONG_CODE);
}

void signin(int sock) {
  if(logged_in) {
    log_warn("You are logged in.");
    return;
  }

  printf("\n===== Sign in =====\n");
  char username_input[MAX_USERNAME];
  input("Username", username_input, MAX_USERNAME, false);
  int num_time_wrong_code, num_time_wrong_password;

  if(!verify_username(sock, username_input, &num_time_wrong_code, &num_time_wrong_password)) {
    err_error(ERR_ACCOUNT_NOT_FOUND);
    return;
  }

  char password_input[MAX_PASSWORD];
  char opt[BUFFER];

  do {
    // Reset password input
    strcpy(password_input, "");
    strcpy(opt, "");

    if(num_time_wrong_password < MAX_WRONG_PASSWORD && num_time_wrong_password > 0) {
      log_warn("You remain %d time(s) input password!", MAX_WRONG_PASSWORD - num_time_wrong_password);
    }
    printf("Password: ");
    char *p = password_input;
    getpasswd (&p, MAX_PASSWORD, '*', stdin);   // Hide password = *
    printf("\n");

    // Check if password is empty
    if(strlen(password_input) == 0) {
      err_error(ERR_INPUT_EMPTY);
      continue;
    }

    http_clear(method, request, response);
    strcpy(method, "POST");
    sprintf(request, "/accounts/authen?data: %s %s", username_input, password_input);
    send_request(sock, method, request);
    int code = get_response(sock, response);
    Account *acc = (Account *) malloc(sizeof *acc);
    char tmp1[BUFFER], tmp2[BUFFER];
    switch (code) {
      // Username and password correct and account is currently active.
      case 202:
        sscanf(response, "202 success %s %s %s %s", acc->username, tmp1, tmp2, acc->homepage);
        logged_in = 1;
        _set_current_user_(*acc);
        secretify(response);
        log_success("Login successfully");
        return;

      // Password incorrect
      case 400:
        err_error(ERR_PASSWORD_INCORRECT);
        sscanf(response, "400 fail %d %d", &num_time_wrong_code, &num_time_wrong_password);
        break;

      // Username and password correct but account is blocked or not active
      case 401:
        sscanf(response, "401 fail %[^\n]s", response);
        log_error("%s", response);
        log_warn("Please activate account or login with other account.");
        return;

      // If user input password incorrect more 3 times -> account blocked
      case 403:
        sscanf(response, "403 fail %d %[^\n]s", &num_time_wrong_password, response);
        remove("secret.txt");
        log_error("%s", response);
        return;
    }

    do {
      input("Would you like to continue? (y/n)", opt, 1, false);
    } while(!(opt[0] == 'y' || opt[0] == 'n'));
    if(opt[0] == 'y') continue;
    else if(opt[0] == 'n') break;

  } while (num_time_wrong_password < MAX_WRONG_PASSWORD);
}

void search(int sock) {
  printf("\n===== Search =====\n");
  char username_input[MAX_USERNAME];
  input("Username", username_input, MAX_USERNAME, false);
  Account *acc = search_account(sock, username_input);

  if(!acc) {
    err_error(ERR_ACCOUNT_NOT_FOUND);
    return;
  }

  char status[50];
  if(acc->status == 0) strcpy(status, "\033[91;1;5mBlocked\033[0m");
  else if(acc->status == 1 || acc->status == -1) strcpy(status, "\x1b[1;38;5;47mActive\x1b[0m");
  else if(acc->status == 2) strcpy(status, "\x1b[1;38;5;226mNot activated\x1b[0m");
  log_success("\n    Username: %s\n    Homepage: %s\n    Status: %s", acc->username, acc->homepage, status);
}

void change_password(int sock) {
  printf("\n===== Change Password =====\n");
  char old_password[MAX_PASSWORD];
  input("Old password", old_password, MAX_PASSWORD, true);

  // Compare password input and old password
  if(!verify_password(sock, curr_user.username, old_password)) {
    err_error(ERR_PASSWORD_INCORRECT);
    return;
  }

  char new_password[MAX_PASSWORD];
  input("New password", new_password, MAX_PASSWORD, true);
  if(strcmp(new_password, old_password) == 0) {
    log_warn("New password equal old password. Please try again...");
    return;
  }

  http_clear(method, request, response);
  strcpy(method, "PATCH");
  sprintf(request, "/accounts/updatePassword?data: %s %s", curr_user.username, new_password);
  send_request(sock, method, request);
  int code = get_response(sock, response);
  if(code == 200) {
    secretify(response);
    log_success("Update password successfully");
    return;
  }
  log_error("%s", response);
}

void signout(int sock) {
  printf("\n===== Sign out =====\n");
  char username_input[MAX_USERNAME];
  input("Username", username_input, MAX_USERNAME, false);

  // Check account if exist
  if(!(strcmp(curr_user.username, username_input) == 0 || strcmp(username_input, "bye") == 0)) {
    log_error("Username is incorrect or different string 'bye'");
    return;
  }

  http_clear(method, request, response);
  strcpy(method, "PATCH");
  sprintf(request, "/accounts/logout?data: %s", curr_user.username);
  send_request(sock, method, request);
  int code = get_response(sock, response);
  if(code == 202) {
    logged_in = 0;
    _reset_current_user_();
    remove("secret.txt");
    log_success("%s", response);
    return;
  }
  log_error("%s", response);
}

void get_domain(int sock) {
  printf("\n===== Homepage Domain =====\n");
  if(!validate_domain_name(curr_user.homepage)) {
    http_clear(method, request, response);
    strcpy(method, "GET");
    sprintf(request, "/accounts/domain/%s", curr_user.homepage);
    send_request(sock, method, request);
    int code = get_response(sock, response);

    char domain[MAX_HOMEPAGE];
    if(code == 200) {
      sscanf(response, "200 success %[^\n]s", domain);
      log_success("Domain name: %s\n", domain);
      return;
    }
    log_warn("%s", response);
    return;
  }
  log_success("Domain name: %s\n", curr_user.homepage);
}

void get_ipv4(int sock) {
  printf("\n===== Homepage IPv4 address =====\n");
  // If homepage is domain_name then convert to ipv4 address ant print
  if(!validate_ip(curr_user.homepage)) {
    http_clear(method, request, response);
    strcpy(method, "GET");
    sprintf(request, "/accounts/ipv4/%s", curr_user.homepage);
    send_request(sock, method, request);
    int code = get_response(sock, response);

    char ipv4List[MAX_HOMEPAGE];
    if(code == 200) {
      sscanf(response, "200 success %[^\n]s", ipv4List);
      log_success("IPv4 address: %s\n", ipv4List);
      return;
    }
    log_warn("%s", response);
    return;
  }

  log_success("IPv4 address: %s\n", curr_user.homepage);
}