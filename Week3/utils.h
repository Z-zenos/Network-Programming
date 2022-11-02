#include <stdio.h>
#include <stdbool.h>
#include "linkedlist.h"

void clear_buffer();
ssize_t getpasswd(char**, size_t, int, FILE*);
void loading();
void load_data(XOR_LL*);
void input(char*, char*, int, bool);
char *strtrim(char*);
void save_data(XOR_LL);