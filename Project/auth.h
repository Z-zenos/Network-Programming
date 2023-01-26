#pragma once

#ifndef _AUTH_H_
#define _AUTH_H_

#include <mysql/mysql.h>
#include "player.h"

#include "http.h"

int signin(MYSQL *, ClientAddr, PlayerTree *,Request *, Response *);
int signup(MYSQL *, PlayerTree *, Request *, Response *);
int change_password(MYSQL *, PlayerTree *, Request *, Response *);
int forgot_password();

#endif