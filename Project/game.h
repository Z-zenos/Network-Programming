#pragma once

#ifndef _GAME_H_
#define _GAME_H_

#include <time.h>

#include "config.h"


typedef struct Game {
  int game_id;
  int board[BOARD_S][BOARD_S];
  Player *player1;
  Player *player2;

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
  int chat_id;
} *Game;

int create_game();
Game find_game(int game_id);

#endif