
#ifndef _CHAT_H_
#define _CHAT_H_

#include "config.h"
#include "game.h"
#include "http.h"
#include "player.h"

int chat(MYSQL *, ClientAddr, GameTree *, PlayerTree *, Message *, int *);

#endif