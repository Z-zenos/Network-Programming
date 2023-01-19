#pragma once

#ifndef PLAYER_H
#define PLAYER_H

#include <mysql/mysql.h>

typedef struct Achievement {
  int win;
  int draw;
  int loss;
  int streak;
  int rank;
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
PlayerTree *load_players(MYSQL *);
//void drop_playertree(PlayerTree *);
int add_player(PlayerTree *, Player);
//int player_delete(PlayerTree *, int);
//Game *player_find(PlayerTree *, int);
void player_info(PlayerTree *);

void rank(PlayerTree *, Message, char *);


#endif