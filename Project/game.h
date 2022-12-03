#pragma once

#ifndef __GAME_H__
#define GAME_H_


typedef struct Game {
  int id;
  int board[SIZE][SIZE];
  Player *player1;
  Player *player2;
  int views;
  char turn; // 'X' or 'Y'
  int num_move;
} *Game;

typedef struct NodeGame {
  char status;
  Game game;

  struct NodeGame *left;
  struct NodeGame *right;
} *NodeGame;

#endif