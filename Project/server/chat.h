
#ifndef _CHAT_H_
#define _CHAT_H_

#include "game.h"
#include "config.h"
#include "http.h"
#include "player.h"

int chat(MYSQL *, ClientAddr, GameTree *, PlayerTree *, Message *, int *);

#endif