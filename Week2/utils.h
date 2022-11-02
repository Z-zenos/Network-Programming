#include <stdio.h>
#include <stdbool.h>

void clear_buffer();
ssize_t getpasswd(char**, size_t, int, FILE*);
void loading();
void input(char*, char*, int, bool);
char *strtrim(char*);
bool is_number(const char*);
int parse_arguments(int, char**, char*);