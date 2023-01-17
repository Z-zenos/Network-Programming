// Example.

#include "rbtree.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

typedef struct student {
  char name[20];
  int age;
} student_t;

typedef struct rbtree studentset_t;

static int student_cmp(const void *p1, const void *p2) {
  student_t *student1, *student2;

  student1 = (student_t *)p1;
  student2 = (student_t *)p2;

  return strcmp(student1->name, student2->name);
}

static void *student_dup(void *p) {
  void *dup_p;

  dup_p = calloc(1, sizeof(struct student));
  memmove(dup_p, p, sizeof(struct student));

  return dup_p;
}

static void student_rel(void *p) { free(p); }

studentset_t *studentset_new() {
  rbtree_t *rbtree;
  rbtree = rbnew(student_cmp, student_dup, student_rel);

  return rbtree;
}

void studentset_delete(studentset_t *studentset) {
  rbdelete(studentset);
}

int studentset_weighted_insert(studentset_t *studentset, int age, char *name) {
  int ret;

  student_t *student;
  student = calloc(1, sizeof(student_t));

  student->age = age;
  strcpy(student->name, name);

  ret = rbinsert(studentset, (void *)student);
  if (ret == 0) {
    printf("failed to insert the student with age %d and weight %s\n", age, name);
    free(student);
    return -1;
  }

  return 0;
}

int studentset_insert(studentset_t *studentset, char *name) {
  return studentset_weighted_insert(studentset, 22, name);
}

int studentset_erase(studentset_t *studentset, char *name) {
  int ret;
  student_t *student;

  student = calloc(1, sizeof(student_t));
  strcpy(student->name, name);

  ret = rberase(studentset, (void *)student);
  if (ret == 0) {
    printf("failed to erase the student with age %s\n", name);
    free(student);
    return -1;
  }

  return 0;
}

double studentset_find(studentset_t *studentset, char *name) {
  student_t *student, student_find;

  strcpy(student_find.name, name);
  student = rbfind(studentset, &student_find);
  if (!student) {
    return NAN;
  }
  return student->age;
}

void studentset_printset(studentset_t *studentset) {
  student_t *student;

  rbtrav_t *rbtrav;
  rbtrav = rbtnew();

  student = rbtfirst(rbtrav, studentset);
  printf("age %d - name: %s\n", student->age, student->name);

  while ((student = rbtnext(rbtrav)) != NULL) {
    printf("age %d - name: %s\n", student->age, student->name);
  }
}

int main() {
  studentset_t *studentset;
  studentset = studentset_new();
  studentset_insert(studentset, "Anh Tuan");
  studentset_insert(studentset, "Lan Anh");
  studentset_insert(studentset, "Tuan Thanh");
  studentset_insert(studentset, "Quang Linh");
  studentset_insert(studentset, "Viet Hung");
  studentset_insert(studentset, "Trong Nghia");
//  double ret;
//  ret = studentset_find(studentset, "Anh Tuan");
//  printf("find 1.0: %f\n", ret);
//  ret = studentset_find(studentset, 8);
//  printf("find 8: %f\n", ret);
//  ret = studentset_erase(studentset, 2);
//  printf("erase 2: %f\n", ret);
//  ret = studentset_find(studentset, 2);
//  printf("find 2: %f\n", ret);
  studentset_printset(studentset);
  studentset_delete(studentset);
  return 0;
}
