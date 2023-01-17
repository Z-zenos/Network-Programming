#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <mysql/mysql.h>
#include <ctype.h>
#include <openssl/evp.h>
#include <openssl/sha.h>

#include "auth.h"
#include "utils.h"
#include "config.h"
#include "notify.h"
#include "log.h"
#include "http.h"



bool is_valid_username(char *username) {
  int i, username_l = strlen(username);
  if(username_l < 4 || username_l > USERNAME_L)  return FAILURE;
  for(i = 0; i < username_l; i++) {
    if(!isalnum(username[i]))
      return FAILURE;
  }
  return SUCCESS;
}

bool is_valid_password(char *password) {
  int i, password_l = strlen(password);
  if(password_l < 4 || password_l > PASSWORD_L) return FAILURE;
  for(i = 0; i < password_l; i++) {
    if(password[i] < 33 || password[i] > 126) return FAILURE;
  }
  return SUCCESS;
}

char *encrypt(char *password) {
  SHA256_CTX context;
  unsigned char md[SHA256_DIGEST_LENGTH];
  SHA256_Init(&context);
  SHA256_Update(&context, (unsigned char*)password, strlen(password));
  SHA256_Final(md, &context);
  char *converter = (char*)malloc(63);
  int k = 0;
  for(i = 0; i < sizeof(pwd_encrypted); i++) {
    k += sprintf(converter + k, "%x", pwd_encrypted[i]);
  }
  return converter;
}

bool compare_password(char *password_input, unsigned char *password_db) {
  SHA256_CTX context;
  unsigned char md[SHA256_DIGEST_LENGTH];
  int pl = strlen(password_input);
  SHA256_Init(&context);
  SHA256_Update(&context, (unsigned char *)password_input, pl);
  SHA256_Final(md, &context);

  int i;
  for(i = 0; i < sizeof(md); i++) {
    if(md[i] != password_db[i])
      return FAILURE;
  }

  return SUCCESS;
}

int signup(MYSQL *conn, Message msg, char *res) {
  char username[USERNAME_L], password[PASSWORD_L];

  // TODO: Get username, password from client
  sscanf(msg.header.params, "username=%[^&]s&password=%[^\r]s", username, password);

  // TODO: Validate
  if(!is_valid_username(username) || !is_valid_password(password)) {
    strcpy(res, "Username / Password invalid");
    return FAILURE;
  }

  // TODO: Check unique username
  char query[QUERY_L];
  sprintf(
    query,
    "SELECT username FROM users WHERE username = '%s'",
    username
  );

  if (mysql_query(conn, query)) {
    notify("error", N_QUERY_FAILED);
    log_error("%s", mysql_error(conn));
    return FAILURE;
  }

  MYSQL_RES *qres = mysql_store_result(conn);
  if(qres->row_count) {
    notify("error", N_USERNAME_ALREADY_EXISTS);
    mysql_free_result(qres);
    strcpy(res, "Username already exists");
    return FAILURE;
  }

  mysql_free_result(qres);

  // TODO: Encrypt password
  char *pwd_encrypted = encrypt(password);

  // TODO: Insert data
  str_clear(query);
  sprintf(
    query,
    "INSERT INTO users(username, password) VALUES('%s', '%s')",
    username, pwd_encrypted
  );

  if (mysql_query(conn, query)) {
    notify("error", N_DATABASE_INSERT_FAILED);
    log_error("%s", mysql_error(conn));
    strcpy(res, "Create new account failed");
    return FAILURE;
  }

  notify("success", N_DATABASE_INSERT_SUCCESS);
  strcpy(res, "Create new account successfully");
  return SUCCESS;
}

int signin(MYSQL *conn, MESSAGE msg, char *res) {
  // TESTING
  char username[USERNAME_L], password[PASSWORD_L];

  // TODO: Get username, password from client
  sscanf(msg.header.params, "username=%[^&]s&password=%[^\r]s", username, password);

  // TODO: Validate
  if(!is_valid_username(username) || !is_valid_password(password)) {
    strcpy(res, "Username / Password invalid");
    return FAILURE;
  }

  char *pwd_encrypted = encrypt(password);

  // TODO: Authen account
  char query[QUERY_L];
  sprintf(
    query,
    "SELECT username, password FROM users WHERE username = '%s' AND password = '%s'",
    username, pwd_encrypted
  );

  if (mysql_query(conn, query)) {
    notify("error", N_QUERY_FAILED);
    log_error("%s", mysql_error(conn));
    return FAILURE;
  }

  MYSQL_RES *qres = mysql_store_result(conn);
  if(!qres->row_count) {
    notify("error", N_ACCOUNT_WRONG);
    mysql_free_result(qres);
    strcpy(res, "Account does not exist");
    return FAILURE;
  }

  mysql_free_result(qres);
  notify("success", N_SIGNIN_SUCCESS);
  strcpy(res, "Login successfully");
  return SUCCESS;
}

//int signout();
int change_password();
int forgot_password();