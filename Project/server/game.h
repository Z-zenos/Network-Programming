
#ifndef _GAME_H_
#define _GAME_H_

#include <mysql/mysql.h>
#include <time.h>

#include "config.h"
#include "http.h"

typedef struct Game {
  int id;
  int player1_id;
  int player2_id;

  char password[PASSWORD_L];
  int num_move;

  /*
                start     player1 win   player2_win   draw
    result:      -1       player1_id    player2_id     0
  */
  int result;
} Game;

#pragma once
typedef struct rbtree GameTree;

#include "player.h"

GameTree *game_new();
void game_drop(GameTree *);
int game_add(GameTree *, Game);
int game_delete(GameTree *, int);
Game *game_find(GameTree *, int);

int caro(MYSQL *, ClientAddr, GameTree *, PlayerTree *, Message *, int *);
int game_create(MYSQL *, ClientAddr, GameTree *, PlayerTree *, Message *, int *);
int game_quick(MYSQL *, ClientAddr, GameTree *, PlayerTree *, Message *, int *);
int game_finish(MYSQL *, ClientAddr, GameTree *, PlayerTree *, Message *, int *);
int game_cancel(MYSQL *, ClientAddr, GameTree *, PlayerTree *, Message *, int *);
int game_list(MYSQL *, ClientAddr, GameTree *, PlayerTree *, Message *, int *);
int game_join(MYSQL *, ClientAddr, GameTree *, PlayerTree *, Message *, int *);
int game_quit(MYSQL *, ClientAddr, GameTree *, PlayerTree *, Message *, int *);

int draw_request(MYSQL *, ClientAddr, GameTree *, PlayerTree *, Message *, int *);
int draw_handler(MYSQL *, ClientAddr, GameTree *, PlayerTree *, Message *, int *);

int duel_request(MYSQL *, ClientAddr, GameTree *, PlayerTree *, Message *, int *);
int duel_handler(MYSQL *, ClientAddr, GameTree *, PlayerTree *, Message *, int *);

#endif