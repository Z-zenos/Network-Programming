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
  char *converter = (char*)malloc(64);
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

int signup(MYSQL *conn, ClientAddr clnt_addr, GameTree *gametree, PlayerTree *playertree, Message *msg, int *receiver) {
  char username[USERNAME_L], password[PASSWORD_L], avatar[AVATAR_L];
  strcpy(username, map_val(msg->params, "username"));
  strcpy(password, map_val(msg->params, "password"));
  strcpy(username, map_val(msg->params, "username"));


  // TODO: Validate
  if(!is_valid_username(username) || !is_valid_password(password)) {
    responsify(msg, "register_fail", NULL);
    return FAILURE;
  }

  // TODO: Check unique username
  char query[QUERY_L];
  sprintf(query, "SELECT username FROM players WHERE username = '%s'", username);

  if (mysql_query(conn, query)) {
    server_error(msg);
    return FAILURE;
  }

  MYSQL_RES *qres = mysql_store_result(conn);
  if(qres->row_count) {
    mysql_free_result(qres);
    responsify(msg, "username_duplicate", NULL);
    return FAILURE;
  }

  mysql_free_result(qres);

  // TODO: Encrypt password
  char *pwd_encrypted = encrypt(password);

  // TODO: Insert data
  str_clear(query);
  sprintf(query, "INSERT INTO players(username, password, avatar) VALUES('%s', '%s', '%s')", username, pwd_encrypted, avatar);

  if (mysql_query(conn, query)) {
    responsify(msg, "register_fail", NULL);
    return FAILURE;
  }

  playertree = player_build(conn);
  char dataStr[DATA_L];
  memset(dataStr, '\0', DATA_L);

  sprintf(
    dataStr,
    "id=%d;username=%s;password=%s;avatar=%s;game=%d;win=%d;draw=%d;loss=%d;points=%d;rank=%d",
    (int)mysql_insert_id(conn), username, pwd_encrypted, avatar, 0, 0, 0, 0, 0, 0
  );

  responsify(msg, "register_success", dataStr);
  return SUCCESS;
}

int signin(MYSQL *conn, ClientAddr clnt_addr, GameTree *gametree, PlayerTree *playertree, Message *msg, int *receiver) {
  int player_id;
  char username[USERNAME_L], password[PASSWORD_L];
  char dataStr[DATA_L];
  memset(dataStr, '\0', DATA_L);
  strcpy(username, map_val(msg->params, "username"));
  strcpy(password, map_val(msg->params, "password"));

  // TODO: Validate username & password
  if(!is_valid_username(username) || !is_valid_password(password)) {
    sprintf(dataStr, "username=%s;password=%s", username, password);
    responsify(msg, "account_incorrect", dataStr);
    return FAILURE;
  }

  char *pwd_encrypted = encrypt(password);

  // TODO: Authen account
  char query[QUERY_L];
  memset(query, '\0', QUERY_L);
  sprintf(query, "SELECT * FROM players WHERE username = '%s' AND password = '%s'", username, pwd_encrypted);

  if (mysql_query(conn, query)) {
    server_error(msg);
    return FAILURE;
  }

  MYSQL_RES *qres = mysql_store_result(conn);
  if(!qres->row_count) {
    mysql_free_result(qres);
    responsify(msg, "account_incorrect", NULL);
    return FAILURE;
  }

  // TODO: Check if the account is logged in on other device?
  MYSQL_ROW row = mysql_fetch_row(qres);
  player_id = atoi(row[0]);

  Player *player_found = player_find(playertree, player_id);
  if(!player_found->sock) player_found->sock = clnt_addr.sock;
  else {
    sprintf(dataStr, "username=%s;password=%s", username, password);
    mysql_free_result(qres);
    responsify(msg, "login_duplicate", dataStr);
    return FAILURE;
  }

  int rank = my_rank(conn, player_id, dataStr);
  memset(dataStr, '\0', DATA_L);

  sprintf(
    dataStr,
    "id=%d;username=%s;password=%s;avatar=%s;game=%d;win=%d;draw=%d;loss=%d;points=%d;rank=%d",
    player_found->id, username, pwd_encrypted, player_found->avatar, player_found->game, player_found->achievement.win,
    player_found->achievement.draw, player_found->achievement.loss, player_found->achievement.points, rank
  );

  mysql_free_result(qres);
  responsify(msg, "login_success", dataStr);
  return SUCCESS;
}

int change_password(MYSQL *conn, ClientAddr clnt_addr, GameTree *gametree, PlayerTree *playertree, Message *msg, int *receiver) {
  int player_id = atoi(map_val(msg->params, "player_id"));
  char old_password[PASSWORD_L], new_password[PASSWORD_L];
  strcpy(old_password, map_val(msg->params, "old_password"));
  strcpy(new_password, map_val(msg->params, "new_password"));

  char dataStr[DATA_L];
  memset(dataStr, '\0', DATA_L);

  // TODO: Encrypt password
  char *old_pwd_encrypted = encrypt(old_password);

  // TODO: Validate old password
  char query[QUERY_L];
  memset(query, '\0', QUERY_L);
  sprintf(query, "SELECT * FROM players WHERE id = %d AND password = '%s'", player_id, old_pwd_encrypted);

  if (mysql_query(conn, query)) {
    server_error(msg);
    return FAILURE;
  }

  MYSQL_RES *qres = mysql_store_result(conn);
  if(!qres->row_count) {
    mysql_free_result(qres);
    responsify(msg, "account_incorrect", NULL);
    return FAILURE;
  }

  // TODO: Validate new password
  if(!is_valid_password(new_password)) {
    responsify(msg, "password_invalid", NULL);
    return FAILURE;
  }

  // TODO: Encrypt new password and update database
  char *new_pwd_encrypted = encrypt(new_password);
  memset(query, '\0', QUERY_L);
  sprintf(query, "UPDATE players SET password = '%s' WHERE id = %d", new_pwd_encrypted, player_id);

  if (mysql_query(conn, query)) {
    server_error(msg);
    return FAILURE;
  }

  playertree = player_build(conn);
  mysql_free_result(qres);
  sprintf(dataStr, "password=%s", new_pwd_encrypted);
  responsify(msg, "password_updated", NULL);
  return SUCCESS;
}

int forgot_password(MYSQL *conn, ClientAddr clnt_addr, GameTree *gametree, PlayerTree * playertree, Message *msg, int *receiver) {
  return SUCCESS;
}