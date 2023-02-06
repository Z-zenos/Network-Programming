#pragma once

#ifndef _AUTH_H_
#define _AUTH_H_

#include <mysql/mysql.h>

#include "game.h"
#include "http.h"
#include "player.h"

int signin(MYSQL *, ClientAddr, GameTree *, PlayerTree *, Message *, int *);
int signout(MYSQL *, ClientAddr, GameTree *, PlayerTree *, Message *, int *);
int signup(MYSQL *, ClientAddr, GameTree *, PlayerTree *, Message *, int *);
int change_password(MYSQL *, ClientAddr, GameTree *, PlayerTree *, Message *, int *);
int forgot_password(MYSQL *, ClientAddr, GameTree *, PlayerTree *, Message *, int *);

#endif