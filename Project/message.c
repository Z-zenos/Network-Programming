#include <stdio.h>
#include <string.h>

#include "message.h"
#include "env.h"

struct T3Message {
  Message messageCode;
  char *message;
};

struct T3Message messages[NUMBER_MESSAGES] = {
  // FAILURE
  { T3_INPUT_EMPTY, "Input empty" },
  { T3_INPUT_INVALID, "Input invalid" },
  { T3_INPUT_TOO_LONG, "Input too long" },
  { T3_EMAIL_INVALID, "Email invalid" },

  { T3_DATABASE_CONNECT_FAILED, "Connect to database failed" },
  { T3_DATABASE_INSERT_FAILED, "Insert into database failed" },
  { T3_USERNAME_ALREADY_EXISTS, "Username already exists" },
  { T3_ACCOUNT_WRONG, "Username or password doesn't correct" },
  { T3_QUERY_FAILED, "Query to database failed" },
  { T3_SIGNIN_SUCCESS, "Sign in successfully" },

  { T3_SERVER_NOT_FOUND, "Server not found" },
  { T3_INIT_CONNECT_FAILED, "Client initializes connect failed" },
  { T3_SEND_REQUEST_FAILED, "Client send request failed" },
  { T3_REQUEST_TOO_LONG, "Request too long" },
  { T3_SERVER_CREATE_SOCKET_FAILED, "Server create socket failed" },
  { T3_SERVER_BIND_SOCKET_FAILED, "Server bind socket to server address failed" },
  { T3_SERVER_LISTEN_CONNECTION_FAILED, "Server listen connection failed" },
  { T3_REQUEST_REJECTED, "Server rejects connection from client" },

  // SUCCESS
  { T3_DATABASE_INSERT_SUCCESS, "Insert data successfully" },
};

void t3_message(char *type, Message msgCode) {
  for(int i = 0; i < NUMBER_MESSAGES; i++) {
    if(messages[i].messageCode == msgCode) {
      if(strcmp(type, "error") == 0)
        printf("\x1b[1;38;5;196mError\t%s\x1b[0m\n", messages[i].message);
      else if(strcmp(type, "success") == 0)
        printf("\x1b[1;38;5;47mSuccess\t%s\x1b[0m\n", messages[i].message);
      else if(strcmp(type, "warn") == 0)
        printf("\x1b[1;38;5;226mWarn\t%s\x1b[0m\n", messages[i].message);
    }
  }
}

