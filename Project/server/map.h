#pragma once

#ifndef _MAP_H_
#define _MAP_H_

#include "config.h"
#include "map.h"

typedef struct Object {
  char key[KEY_L];
  char value[VAL_L];
} Object;

typedef struct rbtree Map;

Map *map_new();
void map_drop(Map *);
int map_add(Map *, Object);
char *map_val(Map *, char *);

#endif