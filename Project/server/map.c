#include <string.h>
#include <stdlib.h>

#include "config.h"
#include "utils.h"
#include "rbtree.h"
#include "map.h"

static int object_cmp(const void *p1, const void *p2) {
  Object *object1, *object2;

  object1 = (Object *)p1;
  object2 = (Object *)p2;

  if(strcmp(object1->key, object2->key) == 1) return 1;
  if(strcmp(object1->key, object2->key) == -1) return -1;
  else return 0;
}

static void *object_dup(void *p) {
  void *dup_p;

  dup_p = calloc(1, sizeof(Object));
  memmove(dup_p, p, sizeof(Object));

  return dup_p;
}

static void object_rel(void *p) { free(p); }

Map *map_new() {
  rbtree_t *rbtree;
  rbtree = rbnew(object_cmp, object_dup, object_rel);
  return rbtree;
}

void map_drop(Map *map) { rbdelete(map); }

int map_add(Map *map, Object new_object) {
  int ret;

  Object *object;
  object = calloc(1, sizeof(Object));
  strcpy(object->key, new_object.key);
  strcpy(object->value, new_object.value);

  ret = rbinsert(map, (void *)object);
  if (ret == 0) {
    free(object);
    return -1;
  }

  return 0;
}

int map_delete(Map *map, char *key) {
  int ret;
  Object *object;

  object = calloc(1, sizeof(Object));
  strcpy(object->key, key);

  ret = rberase(map, (void *)object);
  if (ret == 0) {
    free(object);
    return -1;
  }

  return 0;
}

char *map_val(Map *map, char *key) {
  Object *object, object_find;

  strcpy(object_find.key, key);
  object = rbfind(map, &object_find);

  return !object ? NULL : object->value;
}