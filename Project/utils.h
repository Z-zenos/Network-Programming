#include <stdio.h>
#include <stdbool.h>

void clear_buffer();

/* ===== STRING HANDLER ===== */
bool str_has_space(char *);
char *str_trim(char *);
bool str_start_with(char *, char *);
bool is_number(char *);
bool is_valid_email(char *);
bool is_valid_text(char *);
void str_clear(char *);

ssize_t getpasswd(char **, size_t, int, FILE *);
int input(char *, char *, int);
int input_label(char *, char *, char *, int);