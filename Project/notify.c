#include <stdio.h>
#include <string.h>

#include "notify.h"
#include "config.h"

struct Notify_z {
  Notify notifyCode;
  char *notify;
};

struct Notify_z notifies[NOTIFY_L] = {
  // FAILURE
  { N_INPUT_EMPTY, "Input empty" },
  { N_INPUT_INVALID, "Input invalid" },
  { N_INPUT_TOO_LONG, "Input too long" },
  { N_EMAIL_INVALID, "Email invalid" },

  { N_DATABASE_CONNECT_FAILED, "Connect to database failed" },
  { N_DATABASE_INSERT_FAILED, "Insert into database failed" },
  { N_USERNAME_ALREADY_EXISTS, "Username already exists" },
  { N_ACCOUNT_WRONG, "Username or password doesn't correct" },
  { N_QUERY_FAILED, "Query to database failed" },
  { N_SIGNIN_SUCCESS, "Sign in successfully" },

  { N_SERVER_NOT_FOUND, "Server not found" },
  { N_INIT_CONNECT_FAILED, "Client initializes connect failed" },
  { N_SEND_REQUEST_FAILED, "Client send request failed" },
  { N_REQUEST_TOO_LONG, "Request too long" },
  { N_SERVER_CREATE_SOCKET_FAILED, "Server create socket failed" },
  { N_SERVER_BIND_SOCKET_FAILED, "Server bind socket to server address failed" },
  { N_SERVER_LISTEN_CONNECTION_FAILED, "Server listen connection failed" },
  { N_REQUEST_REJECTED, "Server rejects connection from client" },

  // SUCCESS
  { N_DATABASE_INSERT_SUCCESS, "Insert data successfully" },
};

void notify(char *type, Notify ntfCode) {
  for(int i = 0; i < NOTIFY_L; i++) {
    if(notifies[i].notifyCode == ntfCode) {
      if(strcmp(type, "error") == 0)
        printf("\x1b[1;38;5;196mError\t%s\x1b[0m\n", notifies[i].notify);
      else if(strcmp(type, "success") == 0)
        printf("\x1b[1;38;5;47mSuccess\t%s\x1b[0m\n", notifies[i].notify);
      else if(strcmp(type, "warn") == 0)
        printf("\x1b[1;38;5;226mWarn\t%s\x1b[0m\n", notifies[i].notify);
    }
  }
}

