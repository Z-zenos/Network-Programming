#pragma once

#ifndef _CHAT_H_
#define _CHAT_H_

#include "clist.h"
#include "config.h"
#include "http.h"
#include "game.h"
#include "player.h"

typedef struct Word {
  char word[VAL_L];
} Word;

typedef struct Chat {
  char content[DATA_L];
} Chat;

typedef struct rbtree BadWordsStorage;

BadWordsStorage *bad_words_build();
void bad_words_drop(BadWordsStorage *);
int bad_words_add(BadWordsStorage *, char *);
char *bad_words_find(BadWordsStorage *, char *);

char *bad_words_filter(BadWordsStorage *, char *);
int chat(GameTree *, PlayerTree *, CList *, BadWordsStorage *, Message *, int *);

#endif