#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mysql/mysql.h>
#include <ctype.h>

#include "player.h"
#include "http.h"
#include "config.h"
#include "rbtree.h"
#include "utils.h"

static int player_cmp(const void *p1, const void *p2) {
  Player *player1, *player2;

  player1 = (Player *)p1;
  player2 = (Player *)p2;

  if(player1->id > player2->id) return 1;
  else if(player1->id < player2->id) return -1;
  else return 0;
}

static void *player_dup(void *p) {
  void *dup_p;

  dup_p = calloc(1, sizeof(Player));
  memmove(dup_p, p, sizeof(Player));

  return dup_p;
}

static void player_rel(void *p) { free(p); }

void player_drop(PlayerTree *playertree) { rbdelete(playertree); }

int player_add(PlayerTree *playertree, Player new_player) {
  int ret;

  Player *player;
  player = calloc(1, sizeof(Player));

  player->id = new_player.id;
  strcpy(player->username, new_player.username);
  strcpy(player->password, new_player.password);
  strcpy(player->avatar, new_player.avatar);
  player->game = new_player.game;
  player->sock = new_player.sock;
  player->achievement = new_player.achievement;
  memcpy(player->friends, new_player.friends, FRIEND_L);
  player->is_online = new_player.is_online;
  player->is_playing = new_player.is_playing;

  ret = rbinsert(playertree, (void *)player);
  if (ret == 0) {
    free(player);
    return -1;
  }

  return 0;
}

PlayerTree *player_build(MYSQL *conn) {
  rbtree_t *rbtree;
  rbtree = rbnew(player_cmp, player_dup, player_rel);

  // TODO: Read players data from database
  char query_player[QUERY_L] = "SELECT * FROM players";
  char query_friend[QUERY_L];

  if (mysql_query(conn, query_player)) {
    logger(L_INFO, "Build playertree failed...");
    return NULL;
  }

  MYSQL_RES *res_player = mysql_store_result(conn);
  if(!res_player->row_count) {
    mysql_free_result(res_player);
    logger(L_INFO, "Build playertree failed...");
    return NULL;
  }

  MYSQL_RES *res_friend;
  MYSQL_ROW row_player, row_friend;
  Player player;
  int j = 0;

  while ((row_player = mysql_fetch_row(res_player))) {
    j = 0;
    player.id = atoi(row_player[0]);
    strcpy(player.username, row_player[1]);
    strcpy(player.password, row_player[2]);
    strcpy(player.avatar, row_player[3]);
    player.game = atoi(row_player[4]);
    player.achievement.win = atoi(row_player[5]);
    player.achievement.loss = atoi(row_player[6]);
    player.achievement.draw = atoi(row_player[7]);
    player.achievement.streak = atoi(row_player[8]);
    player.achievement.points = atoi(row_player[9]);
    player.sock = 0;
    player.is_playing = false;
    player.is_online = false;

    memset(query_friend, '\0', QUERY_L);
    sprintf(
      query_friend,
      "SELECT IF(friends.player_id = %d, friends.friend_id, friends.player_id) AS friend_id "
      "FROM players, friends "
      "WHERE players.id IN (friends.player_id, friends.friend_id) AND confirmed = 1 AND players.id = %d",
      player.id, player.id
    );

    mysql_query(conn, query_friend);
    res_friend = mysql_store_result(conn);

    if(res_friend->row_count) {
      while ((row_friend = mysql_fetch_row(res_friend))) {
        player.friends[j] = atoi(row_friend[0]);
        j++;
      }
    }

    player_add(rbtree, player);
  }

  mysql_free_result(res_player);
  mysql_free_result(res_friend);
  logger(L_SUCCESS, "Build playertree successfully...");
  return rbtree;
}

Player *player_find(PlayerTree *playertree, int player_id) {
  Player *player, player_find;

  player_find.id = player_id;
  player = rbfind(playertree, &player_find);

  return !player ? NULL : player;
}

int player_fd(PlayerTree *playertree, int player_id) {
  Player *player_found = player_find(playertree, player_id);
  return player_found->sock;
}

char *player_username(PlayerTree *playertree, int player_id) {
  Player *player_found = player_find(playertree, player_id);
  return player_found->username;
}

int my_rank(MYSQL *conn, int player_id, char *dataStr) {
  // TODO: QUERY find rank of specified player id
  char query[QUERY_L] = ""
    "select er.id, er.username, er.avatar, er.game, er.win, er.draw, er.loss, er.points, (@rank := if(@points = points, @rank, if(@points := points, @rank + 1, @rank + 1))) as ranking "
    "from players er cross join (select @rank := 0, @points := -1) params "
    "order by points desc";

  if (mysql_query(conn, query)) {
    return -1;
  }

  MYSQL_RES *qres = mysql_store_result(conn);
  if(!qres->row_count) {
    mysql_free_result(qres);
    return -1;
  }

  // TODO: Count number of records in db
  MYSQL_ROW row;
  char *idStr = itoa(player_id, 10);
  char line[DATA_L];
  memset(line, '\0', DATA_L);

  while ((row = mysql_fetch_row(qres))) {
    if(strcmp(row[0], idStr) == 0) {
      sprintf(line, "username=%s,avatar=%s,game=%d,win=%d,draw=%d,loss=%d,points=%d", row[1], row[2], atoi(row[3]), atoi(row[4]), atoi(row[5]), atoi(row[6]), atoi(row[7]));
      strcat(dataStr, line);
      mysql_free_result(qres);
      return atoi(row[8]);
    }
  }

  mysql_free_result(qres);
  return -1;
}

int rank(MYSQL *conn, Message *msg) {
  int player_id = atoi(map_val(msg->params, "player_id"));

  // TODO: QUERY follow points from database
  char query[QUERY_L] = "SELECT id, username, avatar, win, loss, points FROM players ORDER BY points DESC LIMIT 10";

  if (mysql_query(conn, query)) {
    server_error(msg);
    return FAILURE;
  }

  MYSQL_RES *qres = mysql_store_result(conn);
  if(!qres->row_count) {
    mysql_free_result(qres);
    server_error(msg);
    return FAILURE;
  }

  // TODO: Count number of records in db
  int total_players = mysql_num_rows(qres), i = 0;
  MYSQL_ROW row;
  Player player[total_players];
  char dataStr[DATA_L];
  memset(dataStr, '\0', sizeof dataStr);

  while ((row = mysql_fetch_row(qres))) {
    player[i].id = atoi(row[0]);
    strcpy(player[i].username, row[1]);
    strcpy(player[i].avatar, row[2]);
    player[i].achievement.win = atoi(row[3]);
    player[i].achievement.loss = atoi(row[4]);
    player[i].achievement.points = atoi(row[5]);
    char line[1000];
    sprintf(
      line,
      "id=%d,username=%s,avatar=%s,win=%d,loss=%d,points=%d;",
      player[i].id, player[i].username, player[i].avatar, player[i].achievement.win,
      player[i].achievement.loss, player[i].achievement.points
    );
    strcat(dataStr, line);
    i++;
  }

  mysql_free_result(qres);
  my_rank(conn, player_id, dataStr);
  responsify(msg, "rank", dataStr);
  return SUCCESS;
}

int profile(MYSQL *conn, Message *msg) {
  char key[USERNAME_L];
  char dataStr[DATA_L], tmp[DATA_L];
  memset(dataStr, '\0', DATA_L);
  strcpy(key, map_val(msg->params, "key"));

  // TODO:
  int i, key_l = strlen(key), type = 0;
  for(i = 0; i < key_l; i++) {
    if(isalpha(key[i])) {
      type = 1;
      break;
    }
  }

  // TODO: QUERY
  char query[QUERY_L];
  memset(query, '\0', QUERY_L);

  switch (type) {
    // if key = 0 -> info is player id
    case 0:
      sprintf(query, "SELECT id, username, avatar, game, win, draw, loss, points FROM players WHERE id = %d", atoi(key));
      break;

    // if key = 1 -> info is username
    case 1:
      sprintf(query, "SELECT id, username, avatar, game, win, draw, loss, points FROM players WHERE username = '%s'", key);
      break;
  }

  if (mysql_query(conn, query)) {
    server_error(msg);
    return FAILURE;
  }

  MYSQL_RES *qres = mysql_store_result(conn);
  if(!qres->row_count) {
    responsify(msg, "player_null", NULL);
    mysql_free_result(qres);
    return FAILURE;
  }

  MYSQL_ROW row;
  while ((row = mysql_fetch_row(qres))) {
    sprintf(
      dataStr,
      "username=%s,avatar=%s,game=%d,win=%d,draw=%d,loss=%d,streak=%d,points=%d,rank=%d",
      row[1], row[2], atoi(row[3]), atoi(row[4]), atoi(row[5]), atoi(row[6]), atoi(row[7]), atoi(row[8]), my_rank(conn, type ? atoi(row[0]) : atoi(key), tmp)
    );
  }

  mysql_free_result(qres);
  responsify(msg, "profile", dataStr);
  return SUCCESS;
}

int friend_check(MYSQL *conn, Message *msg) {
  int player_id = atoi(map_val(msg->params, "player_id"));
  int friend_id = atoi(map_val(msg->params, "friend_id"));

  // TODO: QUERY check friend in database
  char query[QUERY_L];
  memset(query, '\0', QUERY_L);
  sprintf(query, "SELECT * FROM friends WHERE player_id = %d AND friend_id = %d AND confirmed = 1", player_id, friend_id);

  if (mysql_query(conn, query)) {
    responsify(msg, "friend_check", "is_friend=0");
    return SUCCESS;
  }

  MYSQL_RES *qres = mysql_store_result(conn);
  if(!qres->row_count) {
    mysql_free_result(qres);
    responsify(msg, "friend_check", "is_friend=0");
    return SUCCESS;
  }

  mysql_free_result(qres);
  responsify(msg, "friend_check", "is_friend=1");
  return SUCCESS;
}

int friend_list(MYSQL *conn, PlayerTree *playertree, Message *msg) {
  int player_id = atoi(map_val(msg->params, "player_id"));

  // TODO: QUERY check friend in database
  char query[QUERY_L];
  memset(query, '\0', QUERY_L);

  sprintf(
    query,
    "SELECT players.id, players.username, players.avatar "
    "FROM "
      "players, "
      "(SELECT "
        "IF(friends.player_id = %d, friends.friend_id, friends.player_id) AS friend_id "
        "FROM players, friends "
        "WHERE players.id IN (friends.player_id, friends.friend_id) AND confirmed = 1 AND players.id = %d"
      ") AS friend_ids "
      "WHERE players.id = friend_ids.friend_id",
    player_id, player_id
  );

  if (mysql_query(conn, query)) {
    server_error(msg);
    return FAILURE;
  }

  MYSQL_RES *qres = mysql_store_result(conn);
  if(!qres->row_count) {
    mysql_free_result(qres);
    responsify(msg, "friend_list_null", NULL);
    return SUCCESS;
  }

  MYSQL_ROW row;
  char line[100], dataStr[DATA_L];
  memset(line, '\0', 100);
  memset(dataStr, '\0', DATA_L);
  Player *friend;

  while ((row = mysql_fetch_row(qres))) {
    friend = player_find(playertree, atoi(row[0]));
    sprintf(
      line,
      "id=%d,username=%s,avatar=%s,is_online=%s,is_playing=%s;",
      atoi(row[0]), row[1], row[2], friend->is_online ? "true" : "false", friend->is_playing ? "true" : "false"
    );
    strcat(dataStr, line);
  }
  dataStr[strlen(dataStr) - 1] = '\0';

  mysql_free_result(qres);
  responsify(msg, "friend_list", dataStr);
  return SUCCESS;
}

int friend_accept(MYSQL *conn, PlayerTree *playertree, Message *msg) {
  int player_id = atoi(map_val(msg->params, "player_id"));
  int friend_id = atoi(map_val(msg->params, "friend_id"));

  // TODO: Update confirmed column in friends table
  char query[QUERY_L];
  memset(query, '\0', QUERY_L);
  sprintf(query, "UPDATE friends SET confirmed = 1 WHERE friend_id = %d AND player_id = %d", player_id, friend_id);

  if (mysql_query(conn, query)) {
    responsify(msg, NULL, NULL);
    return FAILURE;
  }

  Player *me = player_find(playertree, player_id);
  Player *friend = player_find(playertree, friend_id);
  int i = 0;
  for(i = 0; i < FRIEND_L; i++)
    if(me->friends[i] == 0) me->friends[i] = friend_id;
  for(i = 0; i < FRIEND_L; i++)
    if(friend->friends[i] == 0) friend->friends[i] = player_id;

  responsify(msg, NULL, NULL);
  return SUCCESS;
}

int friend_add(MYSQL *conn, PlayerTree *playertree, Message *msg, int *receiver) {
  int player_id = atoi(map_val(msg->params, "player_id"));
  int friend_id = atoi(map_val(msg->params, "friend_id"));

  // TODO: Insert new add friend request with confirmed = 0 to friends table
  char query[QUERY_L];
  memset(query, '\0', QUERY_L);
  sprintf(query, "INSERT INTO friends (player_id, friend_id, confirmed) VALUES (%d, %d, 0)", player_id, friend_id);

  if (mysql_query(conn, query)) {
    responsify(msg, NULL, NULL);
    return FAILURE;
  }

  Player *friend = player_find(playertree, friend_id);
  char dataStr[DATA_L];
  memset(dataStr, '\0', DATA_L);
  sprintf(dataStr, "player_id=%d,username=%s", player_id, player_username(playertree, player_id));
  receiver[0] = friend->sock;
  responsify(msg, "friend_request", dataStr);
  return SUCCESS;
}