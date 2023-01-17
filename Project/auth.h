#pragma once

#ifndef __AUTH_H__
#define __AUTH_H_

#include <mysql/mysql.h>

#include "http.h"

int signin(MYSQL *, Message, char *);
int signup(MYSQL *, Message, char *);
int signout();
int change_password();
int forgot_password();

#endif