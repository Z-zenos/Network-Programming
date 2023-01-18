#pragma once

#ifndef PLAYER_H
#define PLAYER_H


typedef struct Achievement {
  int win;
  int draw;
  int loss;
  int streak;
  int rank;
  int rank_points;
} Achievement;

typedef struct User {
  int id;
  int sock;
  char username[USERNAME_L];
  char password[PASSWORD_L];
  Achievement achievement;
} Player;

#endif