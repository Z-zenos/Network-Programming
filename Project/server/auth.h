#pragma once

#ifndef _AUTH_H_
#define _AUTH_H_

#include <mysql/mysql.h>

#include "game.h"
#include "http.h"
#include "player.h"

int signin(MYSQL *, ClientAddr, PlayerTree *, Message *);
int signout(PlayerTree *, Message *);
int signup(MYSQL *, ClientAddr, PlayerTree *, Message *);
int change_password(MYSQL *, PlayerTree *, Message *);
int forgot_password(MYSQL *, ClientAddr, GameTree *, PlayerTree *, Message *, int *);

#endif