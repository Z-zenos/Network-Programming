#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>

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

size_t strjoin(char *, size_t, const char *, char **);

char *itoa(int, int);
void logger(char *, int, ...);
void time_print(char *, char *, char *, char *, int);
char **str_split(char *, const char);