#pragma once

#ifndef _GAME_H_
#define _GAME_H_

#include <time.h>
#include <mysql/mysql.h>
#include "http.h"
#include "config.h"
#include "player.h"

typedef struct Game {
  int id;
  int player1_id;
  int player2_id;
  int chat_id;

  char board[BOARD_S][BOARD_S];

  int views;
  int spectators[MAX_SPECTATOR];

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

typedef struct rbtree GameTree;

GameTree *game_new();
void game_drop(GameTree *);
int game_add(GameTree *, Game);
int game_delete(GameTree *, int);
Game *game_find(GameTree *, int);
void game_info(GameTree *);
void game_handler(GameTree *, PlayerTree *, Request *, Response *);
void game_create(MYSQL *, GameTree *, Request *, Response *);
void game_view(GameTree *, Request *, Response *);
char *game_board2string(char [BOARD_S][BOARD_S]);
void game_join(GameTree *, Request *, Response *);
void game_quit(GameTree *, Request *, Response *);
#endif