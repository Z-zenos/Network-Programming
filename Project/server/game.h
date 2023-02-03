
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
  int chat_id;

  char board[BOARD_S][BOARD_S];
  char password[PASSWORD_L];

  int views;

  /* joiner array contains socket of player 1, player 2 and 20 spectators */
  int joiner[MAX_SPECTATOR + 2];

  /* current: X or O */
  char turn;

  /* Coordinate of X / O */
  int col;
  int row;

  int num_move;
  time_t time;

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
void game_info(GameTree *);
int game_handler(MYSQL *, ClientAddr, GameTree *, PlayerTree *, Message *, int *);
int game_create(MYSQL *, ClientAddr, GameTree *, PlayerTree *, Message *, int *);
int game_cancel(MYSQL *, ClientAddr, GameTree *, PlayerTree *, Message *, int *);
int game_view(MYSQL *, ClientAddr, GameTree *, PlayerTree *, Message *, int *);
int game_list(MYSQL *, ClientAddr, GameTree *, PlayerTree *, Message *, int *);
char *game_board2string(char [BOARD_S][BOARD_S]);
int game_join(MYSQL *, ClientAddr, GameTree *, PlayerTree *, Message *, int *);
int game_quit(MYSQL *, ClientAddr, GameTree *, PlayerTree *, Message *, int *);

#endif