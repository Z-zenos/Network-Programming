#pragma once

#ifndef __USER_H__
#define __USER_H__

#include "config.h"

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
  char email[EMAIL_L];
  char username[USERNAME_L];
  char password[PASSWORD_L];
  Achievement achievement;
} *User;

int searchUser(char *);

#endif