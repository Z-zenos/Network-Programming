#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "account.h"
#include "error.h"
#include "linkedlist.h"
#include "log.h"
#include "utils.h"

extern int logged_in;
extern Account curr_user;

// Set current user
void _set_current_user_(Account acc) {
  strcpy(curr_user.username, acc.username);
  strcpy(curr_user.password, acc.password);
  curr_user.status = acc.status;
}

// Logout user
void _reset_current_user_() {
  strcpy(curr_user.username, "");
  strcpy(curr_user.password, "");
  curr_user.status = 2;
}

void signup(XOR_LL *ll) {
  if(!ll) return;

  printf("\n===== Register =====\n");

  Account *new_account = NULL;
  char username_input[MAX_USERNAME];
  input("Username", username_input, MAX_USERNAME, false);
  new_account = search_account(*ll, username_input);

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

    // Default status = idle
    new_account->status = 2;

    xor_ll_push_tail(ll, new_account, sizeof *new_account);
    log_success("Register successfully!");
    save_data(*ll);
  }
}

void activate(XOR_LL *ll) {
  printf("\n===== Activate Account =====\n");

  Account *acc = malloc(sizeof(acc));
  input("Username", acc->username, MAX_USERNAME, false);
  acc = search_account(*ll, acc->username);

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
    log_info("Account activated.");
    return;
  }

  char password_input[MAX_PASSWORD];
  input("Password", password_input, MAX_PASSWORD, true);

  if(strcmp(acc->password, password_input) != 0) {
    err_error(ERR_PASSWORD_INCORRECT);
    return;
  }

  int numTimesInputCode = 0;
  char code[BUFFER];
  do {
    // Reset activation code
    strcpy(code, "");
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
      save_data(*ll);
      return;
    }

    err_error(ERR_ACTIVATION_CODE_INCORRECT);
    log_warn("You remain %d time(s) input password!", 4 - ++numTimesInputCode);
  } while(numTimesInputCode < 4);

  // Block account
  acc->status = 0;
  save_data(*ll);
  err_error(ERR_ACCOUNT_BLOCKED);
}

void signin(XOR_LL *ll) {
  if(logged_in) {
    log_warn("You are logged in.");
    return;
  }

  printf("\n===== Sign in =====\n");
  Account *acc = malloc(sizeof *acc);
  input("Username", acc->username, MAX_USERNAME, false);
  acc = search_account(*ll, acc->username);

  if(!acc) {
    err_error(ERR_ACCOUNT_NOT_FOUND);
    return;
  }

  char password_input[MAX_PASSWORD];
  int numInputPassword = 0;

  do {
    // Reset password input
    strcpy(password_input, "");
    printf("Password: ");
    char *p = password_input;
    getpasswd (&p, MAX_PASSWORD, '*', stdin);
    printf("\n");

    // Check if activation code is empty
    if(strlen(password_input) == 0) {
      err_error(ERR_INPUT_EMPTY);
      continue;
    }

    if(strcmp(acc->password, password_input) != 0) {
      err_error(ERR_PASSWORD_INCORRECT);
      log_warn("You remain %d time(s) input password!", 3 - ++numInputPassword);
      continue;
    }
    // If password correct
    else {
      // Check status of account
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

      acc->status = -1;
      _set_current_user_(*acc);
      logged_in = 1;
      save_data(*ll);
      log_success("Log in successfully!");
      return;
    }

  } while (numInputPassword < 3);

  // If user input password incorrect more 3 times -> account blocked
  err_error(ERR_ACCOUNT_BLOCKED);
  acc->status = 0;
  save_data(*ll);
  log_warn("You have entered the wrong password more than 3 times.");
}

Account *search_account(XOR_LL ll, char *username) {
  XOR_LL_ITERATOR itr = XOR_LL_ITERATOR_INITIALISER;

  XOR_LL_LOOP_HTT_RST(&ll, &itr) {
    Account *acc = (Account*)itr.node_data.ptr;
    if(strcmp(acc->username, username) == 0) {
      return acc;
    }
  }

  return NULL;
}

void search(XOR_LL ll) {
  if(!logged_in) {
    err_error(ERR_NON_LOG_IN);
    return;
  }

  printf("\n===== Search =====\n");
  char username_input[MAX_USERNAME];
  input("Username", username_input, MAX_USERNAME, false);
  Account *acc = search_account(ll, username_input);

  if(!acc) {
    err_error(ERR_ACCOUNT_NOT_FOUND);
    return;
  }

  char status[50];
  if(acc->status == 0) strcpy(status, "\033[91;1;5mBlocked\033[0m");
  else if(acc->status == 1 || acc->status == -1) strcpy(status, "\x1b[1;38;5;47mActive\x1b[0m");
  else if(acc->status == 2) strcpy(status, "\x1b[1;38;5;226mNot activated\x1b[0m");
  log_success("\n    Username: %s\n    Status: %s", acc->username, status);
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
  Account *acc = search_account(*ll, curr_user.username);
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

  Account *acc = search_account(*ll, username_input);
  acc->status = 1;
  logged_in = 0;
  save_data(*ll);
  _reset_current_user_();
  log_success("You are logged out.");
}