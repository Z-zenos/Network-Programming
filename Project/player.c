
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mysql/mysql.h>

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

  return rbtree;
}

Player *player_find(PlayerTree *playertree, int player_id) {
  Player *player, player_find;

  player_find.id = player_id;
  player = rbfind(playertree, &player_find);
  if (!player) {
    return 0;
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

void rank(MYSQL *conn, Request *req, Response *res) {
  // TODO: QUERY follow points from database
  char query[QUERY_L] = "SELECT username, win, draw, loss, points FROM players ORDER BY points DESC";

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

  while ((row = mysql_fetch_row(qres))) {
    strcpy(player[i].username, row[0]);
    player[i].achievement.win = atoi(row[1]);
    player[i].achievement.draw = atoi(row[2]);
    player[i].achievement.loss = atoi(row[3]);
    player[i].achievement.points = atoi(row[4]);
    printf("no: %d - username: %s - win: %d - draw: %d - loss: %d - points: %d\n", i + 1, player[i].username, player[i].achievement.win, player[i].achievement.draw, player[i].achievement.loss, player[i].achievement.points);
    i++;
  }

  mysql_free_result(qres);
  responsify(res, 200, "...", "Get rank successfully");
}