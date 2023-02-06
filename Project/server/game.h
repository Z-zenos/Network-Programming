
#ifndef _GAME_H_
#define _GAME_H_

#include <time.h>
#include <mysql/mysql.h>

#include "http.h"
#include "config.h"

typedef struct Game {
  int id;
  int player1_id;
  int player2_id;

  char board[BOARD_S][BOARD_S];
  char password[PASSWORD_L];

  /* current: X or O */
  char turn;

  /* Coordinate of X / O */
  int col;
  int row;

  int num_move;

  /*
   `if player1 win player2 then [result = player1's id]
  ``if both player draw then [result = 0]
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
int game_finish(MYSQL *, ClientAddr, GameTree *, PlayerTree *, Message *, int *);
int caro(MYSQL *, ClientAddr, GameTree *, PlayerTree *, Message *, int *);
int game_create(MYSQL *, ClientAddr, GameTree *, PlayerTree *, Message *, int *);
int game_quick(MYSQL *, ClientAddr, GameTree *, PlayerTree *, Message *, int *);
int game_cancel(MYSQL *, ClientAddr, GameTree *, PlayerTree *, Message *, int *);
int game_list(MYSQL *, ClientAddr, GameTree *, PlayerTree *, Message *, int *);
int game_join(MYSQL *, ClientAddr, GameTree *, PlayerTree *, Message *, int *);
int game_quit(MYSQL *, ClientAddr, GameTree *, PlayerTree *, Message *, int *);

int draw_request(MYSQL *, ClientAddr, GameTree *, PlayerTree *, Message *, int *);
int draw_handler(MYSQL *, ClientAddr, GameTree *, PlayerTree *, Message *, int *);

int duel_request(MYSQL *, ClientAddr, GameTree *, PlayerTree *, Message *, int *);
int duel_handler(MYSQL *, ClientAddr, GameTree *, PlayerTree *, Message *, int *);

#endif