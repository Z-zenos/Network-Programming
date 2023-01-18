#pragma once

#ifndef _GAME_H_
#define _GAME_H_

#include <time.h>

#include "config.h"

typedef struct Game {
  int id;
  int player1_id;
  int player2_id;
  int chat_id;

  int board[BOARD_S][BOARD_S];

  int views;

  /* current: X or O */
  char turn;
  int num_move;
  time_t time;

  /*
   `if player1 win player2 then [result = player1's id]
  ``if both player draw then [result = 0]
  */
  int result;
} Game;

typedef struct rbtree GameTree;

GameTree *game_new();
void game_drop(GameTree *);
int game_add(GameTree *, Game);
int game_delete(GameTree *, int);
int game_find(GameTree *, int);
void game_info(GameTree *);

#endif