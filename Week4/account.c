#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "network.h"
#include "account.h"
#include "error.h"
#include "http.h"
#include "linkedlist.h"
#include "log.h"
#include "utils.h"

extern int logged_in;
extern Account curr_user;

char method[10] = "";
char request[BUFFER] = "";
char response[BUFFER] = "";

// Set current user
void _set_current_user_(Account acc) {
  strcpy(curr_user.username, acc.username);
  strcpy(curr_user.password, acc.password);
  strcpy(curr_user.homepage, acc.homepage);
  curr_user.status = acc.status;
  curr_user.num_time_wrong_password = acc.num_time_wrong_password;
  curr_user.num_time_wrong_code = acc.num_time_wrong_code;
}

// Logout user
void _reset_current_user_() {
  strcpy(curr_user.username, "");
  strcpy(curr_user.password, "");
  strcpy(curr_user.homepage, "");
}

void accountify(Account *acc, char *str) {
  sscanf(str, "%s %s %d %d %d %s", acc->username, acc->password, &acc->status, &acc->num_time_wrong_code, &acc->num_time_wrong_password, acc->homepage);
}

Account *search_account(char *username) {
  http_clear(method, request, response);
  strcpy(method, "GET");
  sprintf(request, "/accounts/%s", username);
  send_request(method, request, response);
  get_response(request, response);
  if(strcmp(response, "") == 0)
    return NULL;
  Account *acc = malloc(sizeof(acc));
  accountify(acc, response);
  return acc;
}

void signup() {

  printf("\n===== Register =====\n");

  Account *new_account = NULL;
  char username_input[MAX_USERNAME];
  input("Username", username_input, MAX_USERNAME, false);
  new_account = search_account(username_input);

  // Check account if exist
  if(new_account) {
    err_error(ERR_ACCOUNT_EXISTED);
    return;
  }
  else {
    // Create new account
    new_account = malloc(sizeof(*new_account));
    if(!new_account) {
      err_error(ERR_MEMORY_FULL);
      return;
    }

    strcpy(new_account->username, username_input);
    input("Password", new_account->password, MAX_PASSWORD, true);
    input("Homepage", new_account->homepage, MAX_HOMEPAGE, false);

    if(!validate_domain_name(new_account->homepage) && !validate_ip(new_account->homepage)) {
      err_error(ERR_INVALID_HOMEPAGE_ADDRESS);
      return;
    }

    // Default status = idle
    new_account->status = 2;
    new_account->num_time_wrong_password = new_account->num_time_wrong_code = 0;

    http_clear(method, request, response);
    strcpy(method, "POST");

    sprintf(
      request,
      "/accounts?data: %s %s %d %d %d %s",
      new_account->username, new_account->password, new_account->status, new_account->num_time_wrong_code, new_account->num_time_wrong_password, new_account->homepage
    );

    send_request(method, request, response);
    get_response(request, response);
    if(strcmp(response, "") == 0) {
      err_error(ERR_REGISTER_ACCOUNT_FAILED);
      return;
    }
    sscanf(response, "201 success %[^\n]s", response);
    log_success("%s", response);
  }
}

void activate(XOR_LL *ll) {
  printf("\n===== Activate Account =====\n");

  Account *acc = malloc(sizeof(acc));
  input("Username", acc->username, MAX_USERNAME, false);
  acc = search_account(acc->username);

  if(!acc) {
    err_error(ERR_ACCOUNT_NOT_FOUND);
    return;
  }

  // Can't activate account yourself
  if(
    strcmp(acc->username, curr_user.username) == 0 ||
    acc->status == 1 ||
    acc->status == -1
  ) {
    log_info("Can't activate account signing in.");
    return;
  }

  char password_input[MAX_PASSWORD];
  input("Password", password_input, MAX_PASSWORD, true);

  if(strcmp(acc->password, password_input) != 0) {
    err_error(ERR_PASSWORD_INCORRECT);
    return;
  }

  // Neu nhap sai 2 lan -> con 1 lan nhap dung xong thoat ra vao lai thi so lan nhap con lai co la 1 khong
  char code[BUFFER];
  char opt[BUFFER];

  do {
    // Reset activation code
    strcpy(code, "");

    // Input activation code
    if(acc->num_time_wrong_code < MAX_WRONG_CODE && acc->num_time_wrong_code > 0) {
      log_warn("You remain %d time(s) input activation code!", MAX_WRONG_CODE - acc->num_time_wrong_code);
    }
    printf("Activation code: ");
    scanf("%[^\n]s", code);
    clear_buffer();

    // Check if activation code is empty
    if(strlen(code) == 0) {
      err_error(ERR_INPUT_EMPTY);
      continue;
    }

    // If activation code is true => change status to active
    if(strcmp(code, ACTIVATION_CODE) == 0) {
      if(acc->status == 1 || acc->status == -1) {
        log_info("Account activated.");
        return;
      }

      log_success("Activate account successfully.");
      acc->status = 1;
      acc->num_time_wrong_code = 0;
      acc->num_time_wrong_password = 0;
      save_data(*ll);
      return;
    }

    err_error(ERR_ACTIVATION_CODE_INCORRECT);
    ++acc->num_time_wrong_code;

    if(acc->num_time_wrong_code == MAX_WRONG_CODE) break;
    save_data(*ll);

    do {
      input("Would you like to continue? (y/n): ", opt, 1, false);
    } while(!(opt[0] == 'y' || opt[0] == 'n'));
    if(opt[0] == 'y') continue;
    else if(opt[0] == 'n') break;

  } while(acc->num_time_wrong_code < MAX_WRONG_CODE);

  // Block account
  if(acc->num_time_wrong_code == MAX_WRONG_CODE) {
    acc->status = 0;
    save_data(*ll);
    err_error(ERR_ACCOUNT_BLOCKED);
  }
}

void signin(XOR_LL *ll) {
  if(logged_in) {
    log_warn("You are logged in.");
    return;
  }

  printf("\n===== Sign in =====\n");
  Account *acc = malloc(sizeof *acc);
  input("Username", acc->username, MAX_USERNAME, false);
  acc = search_account(acc->username);

  // If account not found then return main menu
  if(!acc) {
    err_error(ERR_ACCOUNT_NOT_FOUND);
    return;
  }

  // Check status of account(if account blocked/not activated -> return main menu)
  if(acc->status == 0) {
    err_error(ERR_ACCOUNT_BLOCKED);
    log_warn("Please activate account or login with other accounts.");
    return;
  }

  if(acc->status == 2) {
    err_error(ERR_ACCOUNT_NON_ACTIVATED);
    log_warn("Please activate account or login with other accounts.");
    return;
  }

  char password_input[MAX_PASSWORD];
  char opt[BUFFER];

  do {
    // Reset password input
    strcpy(password_input, "");
    strcpy(opt, "");

    // Input password
    if(acc->num_time_wrong_password < MAX_WRONG_PASSWORD && acc->num_time_wrong_password > 0) {
      log_warn("You remain %d time(s) input password!", MAX_WRONG_PASSWORD - acc->num_time_wrong_password);
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

    // Check user input = password of account
    if(strcmp(acc->password, password_input) == 0) {
      acc->status = -1;
      acc->num_time_wrong_code = 0;
      acc->num_time_wrong_password = 0;
      logged_in = 1;
      _set_current_user_(*acc);
      save_data(*ll);
      log_success("Log in successfully!");
      return;
    }

    err_error(ERR_PASSWORD_INCORRECT);
    ++acc->num_time_wrong_password;

    if(acc->num_time_wrong_password == MAX_WRONG_PASSWORD) break;

    save_data(*ll);
    do {
      input("Would you like to continue? (y/n): ", opt, 1, false);
    } while(!(opt[0] == 'y' || opt[0] == 'n'));

    if(opt[0] == 'y') continue;
    else if(opt[0] == 'n') break;
  } while (acc->num_time_wrong_password < MAX_WRONG_PASSWORD);

  // If user input password incorrect more 3 times -> account blocked
  if(acc->num_time_wrong_password == MAX_WRONG_PASSWORD) {
    acc->status = 0;
    save_data(*ll);
    log_warn("You have entered the wrong password more than 3 times.");
    err_error(ERR_ACCOUNT_BLOCKED);
  }
}

void search(XOR_LL ll) {
  if(!logged_in) {
    err_error(ERR_NON_LOG_IN);
    return;
  }

  printf("\n===== Search =====\n");
  char username_input[MAX_USERNAME];
  input("Username", username_input, MAX_USERNAME, false);
  Account *acc = search_account(username_input);

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

void change_password(XOR_LL *ll) {
  if(!logged_in) {
    err_error(ERR_NON_LOG_IN);
    return;
  }

  printf("\n===== Change Password =====\n");
  char password_input[MAX_PASSWORD];
  input("Old password", password_input, MAX_PASSWORD, true);

  // Compare password input and old password
  if(strcmp(curr_user.password, password_input) != 0) {
    err_error(ERR_PASSWORD_INCORRECT);
    return;
  }

  input("New password", password_input, MAX_PASSWORD, true);
  if(strcmp(password_input, curr_user.password) == 0) {
    log_warn("New password equal old password. Please try again...");
    return;
  }

  Account *acc = search_account(curr_user.username);
  strcpy(acc->password, password_input);
  _set_current_user_(*acc);
  save_data(*ll);
  log_success("Change password successfully.");
}

void signout(XOR_LL *ll) {
  // Check user login-ed
  if(!logged_in) {
    err_error(ERR_NON_LOG_IN);
    return;
  }

  printf("\n===== Sign out =====\n");
  char username_input[MAX_USERNAME];
  input("Username", username_input, MAX_USERNAME, false);

  // Check account if exist
  if(strcmp(curr_user.username, username_input) != 0) {
    err_error(ERR_USERNAME_INCORRECT);
    return;
  }

  Account *acc = search_account(username_input);
  acc->status = 1;
  logged_in = 0;
  save_data(*ll);
  _reset_current_user_();
  log_success("You are logged out.");
}

void get_domain() {
  // Check user login-ed
  if(!logged_in) {
    err_error(ERR_NON_LOG_IN);
    return;
  }

  // If homepage is ipv4 address then convert to hostname ant print
  if(!validate_domain_name(curr_user.homepage)) {
    ip_to_domain_name(curr_user.homepage);
    return;
  }

  log_success("Domain name: %s\n", curr_user.homepage);
}

void get_ipv4() {
  // Check user login-ed
  if(!logged_in) {
    err_error(ERR_NON_LOG_IN);
    return;
  }

  // If homepage is domain_name then convert to ipv4 address ant print
  if(!validate_ip(curr_user.homepage)) {
    domain_name_to_ip(curr_user.homepage);
    return;
  }

  log_success("IPv4 address: %s\n", curr_user.homepage);
}