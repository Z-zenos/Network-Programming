#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mysql/mysql.h>
#include <ctype.h>

#include "notify.h"
#include "player.h"
#include "http.h"
#include "game.h"
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

int player_add(PlayerTree *playertree, Player new_player) {
  int ret;

  Player *player;
  player = calloc(1, sizeof(Player));

  player->id = new_player.id;
  strcpy(player->username, new_player.username);
  strcpy(player->password, new_player.password);
  player->sock = -1;
  player->achievement = new_player.achievement;

  ret = rbinsert(playertree, (void *)player);
  if (ret == 0) {
    logger(L_ERROR, 1, "Can't insert new player for players");
    free(player);
    return -1;
  }

  return 0;
}

PlayerTree *player_build(MYSQL *conn) {
  rbtree_t *rbtree;
  rbtree = rbnew(player_cmp, player_dup, player_rel);

  // TODO: Read players data from database
  char query[QUERY_L] = "SELECT * FROM players";

  if (mysql_query(conn, query)) {
    notify("error", N_QUERY_FAILED);
    logger(L_ERROR, 1, mysql_error(conn));
    return NULL;
  }

  MYSQL_RES *qres = mysql_store_result(conn);
  if(!qres->row_count) {
    notify("error", N_ACCOUNT_WRONG);
    mysql_free_result(qres);
    return NULL;
  }

  MYSQL_ROW row;
  Player player;

  while ((row = mysql_fetch_row(qres))) {
    player.id = atoi(row[0]);
    strcpy(player.username, row[1]);
    strcpy(player.password, row[2]);
    player.achievement.win = atoi(row[3]);
    player.achievement.loss = atoi(row[4]);
    player.achievement.draw = atoi(row[5]);
    player.achievement.streak = atoi(row[6]);
    player.achievement.points = atoi(row[7]);
    player_add(rbtree, player);
  }

  mysql_free_result(qres);
  logger(L_SUCCESS, 1, "Build playertree successfully...");
  return rbtree;
}

Player *player_find(PlayerTree *playertree, int player_id) {
  Player *player, player_find;

  player_find.id = player_id;
  player = rbfind(playertree, &player_find);
  if (!player) {
    return NULL;
  }
  return player;
}

void player_print(Player *player) {
  printf("id %d - username: %s - password: %s - won: %d - draw: %d - loss: %d - streak: %d - points: %d\n", player->id, player->username, player->password, player->achievement.win, player->achievement.draw, player->achievement.loss, player->achievement.streak, player->achievement.points);
}

void player_info(PlayerTree *playertree) {
  Player *player;

  rbtrav_t *rbtrav;
  rbtrav = rbtnew();
  player = rbtfirst(rbtrav, playertree);
  player_print(player);

  while ((player = rbtnext(rbtrav)) != NULL) {
    player_print(player);
  }
}

int my_rank(MYSQL *conn, int player_id, char *dataStr) {
  // TODO: QUERY find rank of specified player id
  char query[QUERY_L] = ""
    "select er.id, er.username, er.win, er.draw, er.loss, er.points, (@rank := if(@points = points, @rank, if(@points := points, @rank + 1, @rank + 1))) as ranking "
    "from players er cross join (select @rank := 0, @points := -1) params "
    "order by points desc";

  if (mysql_query(conn, query)) {
    notify("error", N_QUERY_FAILED);
    logger(L_ERROR, 1, mysql_error(conn));
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
      sprintf(line, "username=%s&win=%d&draw=%d&loss=%d&points=%d", row[1], atoi(row[2]), atoi(row[3]), atoi(row[4]), atoi(row[5]));
      strcat(dataStr, line);
      mysql_free_result(qres);
      return atoi(row[6]);
    }
  }

  mysql_free_result(qres);
  return -1;
}

void rank(MYSQL *conn, Request *req, Response *res) {
  int player_id;

  // TODO: Get player id from request
  if(sscanf(req->header.params, "player_id=%d", &player_id) <= 0) {
    responsify(res, 400, NULL, "Bad request. Usage: GET /rank player_id=...");
    return;
  }

  // TODO: QUERY follow points from database
  char query[QUERY_L] = "SELECT username, win, draw, loss, points FROM players ORDER BY points DESC LIMIT 10";

  if (mysql_query(conn, query)) {
    notify("error", N_QUERY_FAILED);
    logger(L_ERROR, 1, mysql_error(conn));
    return;
  }

  MYSQL_RES *qres = mysql_store_result(conn);
  if(!qres->row_count) {
    mysql_free_result(qres);
    return;
  }

  // TODO: Count number of records in db
  int total_players = mysql_num_rows(qres), i = 0;
  MYSQL_ROW row;
  Player player[total_players];
  char dataStr[DATA_L];
  memset(dataStr, '\0', sizeof dataStr);

  while ((row = mysql_fetch_row(qres))) {
    strcpy(player[i].username, row[0]);
    player[i].achievement.win = atoi(row[1]);
    player[i].achievement.draw = atoi(row[2]);
    player[i].achievement.loss = atoi(row[3]);
    player[i].achievement.points = atoi(row[4]);
    char line[100];
    sprintf(line, "username=%s&win=%d&draw=%d&loss=%d&points=%d;", player[i].username, player[i].achievement.win, player[i].achievement.draw, player[i].achievement.loss, player[i].achievement.points);
    strcat(dataStr, line);
    i++;
  }

  mysql_free_result(qres);
  my_rank(conn, player_id, dataStr);
  responsify(res, 200, dataStr, "Get rank successfully");
}

void profile(MYSQL *conn, Request *req, Response *res) {
  char key[USERNAME_L];
  char msgStr[MESSAGE_L], dataStr[DATA_L], tmp[DATA_L];
  memset(msgStr, '\0', MESSAGE_L);
  memset(dataStr, '\0', DATA_L);

  // TODO: Get key from request
  if(sscanf(req->header.params, "key=%s", key) <= 0) {
    responsify(res, 400, NULL, "Bad request. Usage: GET /profile key=[player_id | username]");
    return;
  }

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
      sprintf(query, "SELECT id, username, win, draw, loss, points FROM players WHERE id = %d", atoi(key));
      break;

    // if key = 1 -> info is username
    case 1:
      sprintf(query, "SELECT id, username, win, draw, loss, points FROM players WHERE username = '%s'", key);
      break;

    default: break;
  }

  if (mysql_query(conn, query)) {
    notify("error", N_QUERY_FAILED);
    logger(L_ERROR, 1, mysql_error(conn));
    return;
  }

  MYSQL_RES *qres = mysql_store_result(conn);
  if(!qres->row_count) {
    sprintf(msgStr, "Player [%s] does not exist", key);
    responsify(res, 400, NULL, msgStr);
    mysql_free_result(qres);
    return;
  }

  MYSQL_ROW row;
  while ((row = mysql_fetch_row(qres))) {
    sprintf(
      dataStr,
      "username=%s&win=%d&draw=%d&loss=%d&streak=%d&points=%d&rank=%d",
      row[1], atoi(row[2]), atoi(row[3]), atoi(row[4]), atoi(row[5]), atoi(row[6]), my_rank(conn, type ? atoi(row[0]) : atoi(key), tmp)
    );
  }

  mysql_free_result(qres);
  sprintf(msgStr, "Get info of player [%s] successfully", key);
  responsify(res, 200, dataStr, msgStr);
}
