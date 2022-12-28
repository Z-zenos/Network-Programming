#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>

#include "auth.h"
#include "utils.h"
#include "env.h"
#include "message.h"
#include "log.h"

int signup(MYSQL *conn) {
  // TESTING
  char username[MAX_LENGTH_USERNAME], password[MAX_LENGTH_PASSWORD], email[MAX_LENGTH_EMAIL];
  if(!input_label("Email", email, "email", MAX_LENGTH_EMAIL)) return FAILURE;
  if(!input_label("Username", username, "text", MAX_LENGTH_USERNAME)) return FAILURE;
  if(!input_label("Password", password, "password", MAX_LENGTH_USERNAME)) return FAILURE;

  // TODO: Get email, username, password from client

  // TODO: Validate

  char query[MAX_LENGTH_QUERY];
  // + Check unique username
  sprintf(
    query,
    "SELECT username FROM users WHERE username = '%s'",
    username
  );

  // + Query failed
  if (mysql_query(conn, query)) {
    t3_message("error", T3_QUERY_FAILED);
    log_error("%s", mysql_error(conn));

    // TODO: Send response to client

    return FAILURE;
  }

  MYSQL_RES *res = mysql_store_result(conn);
  if(res->row_count) {
    t3_message("error", T3_USERNAME_ALREADY_EXISTS);
    mysql_free_result(res);

    // TODO: Send response to client

    return FAILURE;
  }

  mysql_free_result(res);

  // + Insert data
  str_clear(query);
  sprintf(
    query,
    "INSERT INTO users(email, username, password) VALUES('%s', '%s', '%s')",
    email, username, password
  );

  if (mysql_query(conn, query)) {
    t3_message("error", T3_DATABASE_INSERT_FAILED);
    log_error("%s", mysql_error(conn));

    // TODO: Send response to client

    return FAILURE;
  }

  t3_message("success", T3_DATABASE_INSERT_SUCCESS);

  // TODO: Send response to client

  return SUCCESS;
}

int signin(MYSQL *conn) {
  // TESTING
  char username[MAX_LENGTH_USERNAME], password[MAX_LENGTH_PASSWORD];
  if(!input_label("Username", username, "text", MAX_LENGTH_USERNAME)) return FAILURE;
  if(!input_label("Password", password, "password", MAX_LENGTH_USERNAME)) return FAILURE;

  // TODO: Get username, password from client

  // TODO: Validate

  char query[MAX_LENGTH_QUERY];

  // + Check username if exist
  sprintf(
    query,
    "SELECT username, password FROM users WHERE username = '%s' AND password = '%s'",
    username, password
  );

  // + Query failed
  if (mysql_query(conn, query)) {
    t3_message("error", T3_QUERY_FAILED);
    log_error("%s", mysql_error(conn));

    // TODO: Send response to client

    return FAILURE;
  }

  MYSQL_RES *res = mysql_store_result(conn);
  if(!res->row_count) {
    t3_message("error", T3_ACCOUNT_WRONG);
    mysql_free_result(res);

    // TODO: Send response to client

    return FAILURE;
  }

  mysql_free_result(res);
  t3_message("success", T3_SIGNIN_SUCCESS);

  // TODO: Send response to client

  return SUCCESS;
}

int signout();
int change_password();
int forgot_password();