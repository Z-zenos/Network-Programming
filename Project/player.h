#pragma once

#ifndef _PLAYER_H_
#define _PLAYER_H_

#include <mysql/mysql.h>

#include "http.h"

typedef struct Achievement {
  int win;
  int draw;
  int loss;
  int streak;
  int points;
} Achievement;

typedef struct User {
  int id;
  int sock;
  char username[USERNAME_L];
  char password[PASSWORD_L];
  Achievement achievement;
} Player;

typedef struct rbtree PlayerTree;
PlayerTree *player_build(MYSQL *);
//void drop_playertree(PlayerTree *);
int player_add(PlayerTree *, Player);
//int player_delete(PlayerTree *, int);
Player *player_find(PlayerTree *, int);
void player_info(PlayerTree *);

void rank(MYSQL *, Request *, Response *);
void profile(MYSQL *, PlayerTree *, Request *, Response *);

#endif