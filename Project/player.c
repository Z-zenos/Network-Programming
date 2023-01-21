
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mysql/mysql.h>

#include "notify.h"
#include "player.h"
#include "http.h"
#include "log.h"
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
    log_error("Can't insert new player for players");
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
    log_error("%s", mysql_error(conn));
  }

  MYSQL_RES *qres = mysql_store_result(conn);
  if(!qres->row_count) {
    notify("error", N_ACCOUNT_WRONG);
    mysql_free_result(qres);
    return FAILURE;
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

void player_info(PlayerTree *playertree) {
  Player *player;

  rbtrav_t *rbtrav;
  rbtrav = rbtnew();
  player = rbtfirst(rbtrav, playertree);
  printf("id %d - username: %s - password: %s - won: %d - draw: %d - loss: %d - streak: %d - points: %d\n", player->id, player->username, player->password, player->achievement.win, player->achievement.draw, player->achievement.loss, player->achievement.streak, player->achievement.points);

  while ((player = rbtnext(rbtrav)) != NULL) {
    printf("id %d - username: %s - password: %s - won: %d - draw: %d - loss: %d - streak: %d - points: %d\n", player->id, player->username, player->password, player->achievement.win, player->achievement.draw, player->achievement.loss, player->achievement.streak, player->achievement.points);
  }
}