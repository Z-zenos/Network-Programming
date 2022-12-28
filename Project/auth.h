#pragma once

#ifndef __AUTH_H__
#define __AUTH_H_

#include <mysql/mysql.h>

int signin(MYSQL *);
int signup(MYSQL *);
int signout();
int change_password();
int forgot_password();

#endif