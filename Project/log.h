#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#define log_success(...) do{                                                                     \
	fprintf(stdout,"\x1b[1;38;5;47m[\xE2\x9C\x93]  %s\x1b[0m", __VA_ARGS__); \
	fputs("\n",stdout);                                                                            \
} while(0)

#define log_warn(...) do{                                       \
	fprintf(stdout,"\x1b[1;38;5;226m%s\x1b[0m \t", __VA_ARGS__);  \
	fputs("\n",stdout);                                           \
} while(0)

#define log_error(...) do{                                      \
	fprintf(stdout,"\x1b[1;38;5;196mError\x1b[0m\t");             \
	fprintf(stdout,__VA_ARGS__);                                  \
	fputs("\n",stdout);                                           \
} while(0)

#define log_info(...) do{                                                 \
	fprintf(stdout,"\x1b[1;38;5;202m(￣ ▽ ￣) ノ\t%s\x1b[0m\t",__VA_ARGS__); \
  fputs("\n",stdout);                                                     \
} while(0)
