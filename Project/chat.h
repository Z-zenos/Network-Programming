
#ifndef _CHAT_H_
#define _CHAT_H_

#include "game.h"
#include "config.h"
#include "player.h"

typedef struct Chat {
  int id;
  int player1_id;
  int player2_id;

} Chat;

int chat(GameTree *, PlayerTree *, Request *, Response *);