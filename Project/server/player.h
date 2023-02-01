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
  int game;
  char avatar[AVATAR_L];
  char username[USERNAME_L];
  char password[PASSWORD_L];
  int friends[FRIEND_L];
  Achievement achievement;
} Player;

typedef struct rbtree PlayerTree;
PlayerTree *player_build(MYSQL *);
//void drop_playertree(PlayerTree *);
int player_add(PlayerTree *, Player);
//int player_delete(PlayerTree *, int);
Player *player_find(PlayerTree *, int);
void player_info(PlayerTree *);
int player_fd(PlayerTree *, int);
char *player_username(PlayerTree *, int);
int my_rank(MYSQL *, int, char *);
void rank(MYSQL *, Request *, Response *);
void profile(MYSQL *, Request *, Response *);
void friend_check(MYSQL *, Request *, Response *);
void friend_list(MYSQL *, Request *, Response *);
void friend_request(PlayerTree *, Request *, Response *);
void friend_accept(MYSQL *, PlayerTree *, Request *, Response *);

#endif