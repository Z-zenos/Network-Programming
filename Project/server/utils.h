#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>

void logger(char *, const char *, ...);
void time_print(char *, char *, char *, int, char *);

char *itoa(int, int);
char *str_trim(char *);
char **str_split(char *, const char);

void print_arr(char *, int *);