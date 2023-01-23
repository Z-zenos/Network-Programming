#pragma once

#ifndef __AUTH_H__
#define __AUTH_H_

#include <mysql/mysql.h>

#include "http.h"

int signin(MYSQL *, Request *, Response *);
int signup(MYSQL *, Request *, Response *);
int signout();
int change_password();
int forgot_password();

#endif