#pragma once

#ifndef __USER_H__
#define __USER_H__

#include "env.h"

typedef struct Achievement {
  int win;
  int draw;
  int loss;
  int best_winning_streak;
  int rank;
  int rank_points;
} *Achievement;

typedef struct User {
  int id;
  char email[MAX_LENGTH_EMAIL];
  char username[MAX_LENGTH_USERNAME];
  char password[MAX_LENGTH_PASSWORD];
  Achievement achievement;
} *User;

int searchUser(char *);

#endif