#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <mysql/mysql.h>
#include <ctype.h>
#include <openssl/sha.h>

#include "auth.h"
#include "utils.h"
#include "config.h"
#include "player.h"
#include "notify.h"

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
  int i, k = 0;
  for(i = 0; i < sizeof(md); i++) {
    k += sprintf(converter + k, "%x", md[i]);
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

int signup(MYSQL *conn, PlayerTree *playertree, Request *req, Response *res) {
  char username[USERNAME_L], password[PASSWORD_L];

  // TODO: Get username, password from client
  sscanf(req->header.params, "username=%[A-Za-z0-9]&password=%[A-Za-z0-9]", username, password);

  // TODO: Validate
  if(!is_valid_username(username) || !is_valid_password(password)) {
    responsify(res, 400, NULL, "Username / Password incorrect");
    return FAILURE;
  }

  // TODO: Check unique username
  char query[QUERY_L];
  sprintf(
    query,
    "SELECT username FROM players WHERE username = '%s'",
    username
  );

  if (mysql_query(conn, query)) {
    notify("error", N_QUERY_FAILED);
    logger(L_ERROR, 1, mysql_error(conn));
    responsify(res, 400, NULL, "Internal server error");
    return FAILURE;
  }

  MYSQL_RES *qres = mysql_store_result(conn);
  if(qres->row_count) {
    notify("error", N_USERNAME_ALREADY_EXISTS);
    mysql_free_result(qres);
    responsify(res, 400, NULL, "Username already exists");
    return FAILURE;
  }

  mysql_free_result(qres);

  // TODO: Encrypt password
  char *pwd_encrypted = encrypt(password);

  // TODO: Insert data
  str_clear(query);
  sprintf(
    query,
    "INSERT INTO players(username, password) VALUES('%s', '%s')",
    username, pwd_encrypted
  );

  if (mysql_query(conn, query)) {
    notify("error", N_DATABASE_INSERT_FAILED);
    logger(L_ERROR, 1, mysql_error(conn));
    responsify(res, 400, NULL, "Create new account failed");
    return FAILURE;
  }

  playertree = player_build(conn);
  notify("success", N_DATABASE_INSERT_SUCCESS);
  responsify(res, 201, NULL, "Create new account successfully");
  return SUCCESS;
}

int signin(MYSQL *conn, Request *req, Response *res) {
  // TESTING
  char username[USERNAME_L], password[PASSWORD_L];

  // TODO: Get username, password from client
  sscanf(req->header.params, "username=%[A-Za-z0-9]&password=%[A-Za-z0-9]", username, password);

  // TODO: Validate
  if(!is_valid_username(username) || !is_valid_password(password)) {
    responsify(res, 400, NULL, "Username / Password incorrect");
    return FAILURE;
  }

  char *pwd_encrypted = encrypt(password);

  // TODO: Authen account
  char query[QUERY_L];
  sprintf(
    query,
    "SELECT username, password FROM players WHERE username = '%s' AND password = '%s'",
    username, pwd_encrypted
  );

  if (mysql_query(conn, query)) {
    notify("error", N_QUERY_FAILED);
    logger(L_ERROR, 1, mysql_error(conn));
    responsify(res, 400, NULL, "Internal server error");
    return FAILURE;
  }

  MYSQL_RES *qres = mysql_store_result(conn);
  if(!qres->row_count) {
    notify("error", N_ACCOUNT_WRONG);
    mysql_free_result(qres);
    responsify(res, 400, NULL, "Account does not exist");
    return FAILURE;
  }

  mysql_free_result(qres);
  notify("success", N_SIGNIN_SUCCESS);
  responsify(res, 200, NULL, "Login successfully");
  return SUCCESS;
}

int change_password(MYSQL *conn, PlayerTree *playertree, Request *req, Response *res) {
  int player_id;
  char old_password[PASSWORD_L], new_password[PASSWORD_L];
  char dataStr[DATA_L], msgStr[MESSAGE_L];
  memset(old_password, '\0', PASSWORD_L);
  memset(new_password, '\0', PASSWORD_L);
  memset(msgStr, '\0', MESSAGE_L);
  memset(dataStr, '\0', DATA_L);

  // TODO: Get player id, old password and new password
  if(sscanf(req->header.params, "player_id=%d&old_password=%[A-Za-z0-9]&new_password=%[A-Za-z0-9]", &player_id, old_password, new_password) != 3) {
    responsify(res, 400, NULL, "Bad request. Usage: UPDATE /account/updatePassword player_id=...&old_password=...&new_password=...");
    return FAILURE;
  }

  // TODO: Encrypt password
  char *old_pwd_encrypted = encrypt(old_password);

  // TODO: Validate old password
  char query[QUERY_L];
  memset(query, '\0', QUERY_L);
  sprintf(
    query,
    "SELECT * FROM players WHERE id = %d AND password = '%s'",
    player_id, old_pwd_encrypted
  );

  if (mysql_query(conn, query)) {
    notify("error", N_QUERY_FAILED);
    logger(L_ERROR, 1, mysql_error(conn));
    responsify(res, 400, NULL, "Internal server error");
    return FAILURE;
  }

  MYSQL_RES *qres = mysql_store_result(conn);
  if(!qres->row_count) {
    mysql_free_result(qres);
    responsify(res, 400, NULL, "Current password incorrect");
    return FAILURE;
  }

  // TODO: Validate new password
  if(!is_valid_password(new_password)) {
    sprintf(msgStr, "New password invalid. Password must include alpha, digit and have 4 < length < %d", PASSWORD_L);
    responsify(res, 400, NULL, msgStr);
    return FAILURE;
  }

  // TODO: Encrypt new password and update database
  char *new_pwd_encrypted = encrypt(new_password);
  memset(query, '\0', QUERY_L);
  sprintf(
    query,
    "UPDATE players SET password = '%s' WHERE id = %d",
    new_pwd_encrypted, player_id
  );

  if (mysql_query(conn, query)) {
    notify("error", N_QUERY_FAILED);
    logger(L_ERROR, 1, mysql_error(conn));
    responsify(res, 400, NULL, "Internal server error");
    return FAILURE;
  }

  playertree = player_build(conn);
  mysql_free_result(qres);
  responsify(res, 200, NULL, "Update new password successfully");
  return SUCCESS;
}

int forgot_password();