#pragma once

#ifndef PLAYER_H
#define PLAYER_H

typedef struct Player {
  char role;
  int score;
  const User user;
  Game current_game;

} *Player;

#endif