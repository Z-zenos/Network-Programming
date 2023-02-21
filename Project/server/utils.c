#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "config.h"
#include "utils.h"

char *str_trim(char *str) {
  if( str == NULL ) { return NULL; }
  if( str[0] == '\0' ) { return str; }

  size_t len;
  char *frontp = str;
  char *endp = NULL;

  len = strlen(str);
  endp = str + len;

  /*
   * Move the front and back pointers to address the first non-whitespace
   * characters from each end.
   */
  while(isspace((unsigned char) *frontp)) { ++frontp; }
  if( endp != frontp ) {
    while( isspace((unsigned char) *(--endp)) && endp != frontp ) {}
  }

  if( frontp != str && endp == frontp )
    *str = '\0';
  else if( str + len - 1 != endp )
    *(endp + 1) = '\0';

  /*
   * Shift the string so that it starts at str so that if it's dynamically
   * allocated, we can still free it on the returned pointer.  Note the reuse
   * of endp to mean the front of the string buffer now.
   */
  endp = str;
  if( frontp != str ) {
    while( *frontp ) { *endp++ = *frontp++; }
    *endp = '\0';
  }

  return str;
}

char *itoa(int value, int base) {
  char *result = (char *)malloc(10);

  // check that the base if valid
  if (base < 2 || base > 36) { *result = '\0'; return result; }

  char* ptr = result, *ptr1 = result, tmp_char;
  int tmp_value;

  do {
    tmp_value = value;
    value /= base;
    *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
  } while ( value );

  // Apply negative sign
  if (tmp_value < 0) *ptr++ = '-';
  *ptr-- = '\0';
  while(ptr1 < ptr) {
    tmp_char = *ptr;
    *ptr--= *ptr1;
    *ptr1++ = tmp_char;
  }
  return result;
}

void logger(char *type, const char *format, ...) {
  va_list args;

  if(strcmp(type, L_ERROR) == 0)   printf("\x1b[1;38;5;196m[x]  ");
  if(strcmp(type, L_SUCCESS) == 0) printf("\x1b[1;38;5;47m[\xE2\x9C\x93]  ");
  if(strcmp(type, L_WARN) == 0)    printf("\x1b[1;38;5;226m[!]  ");
  if(strcmp(type, L_INFO) == 0)    printf("\x1b[1;38;5;014m[i]  ");

  va_start(args, format);
  vprintf(format, args);
  va_end(args);
  printf("\x1b[0m\n");
  fflush(stdout);
}

void time_print(char *addr, char *cmd, char *params, int nbytes, char *content) {
  time_t now = time(0);
  char req_time[100];
  strftime(req_time, 100, "%Y-%m-%d %H:%M:%S", localtime(&now));
  printf(
    "\x1b[1;38;5;256m%s>\x1b[0m "
    "\x1b[1;38;5;202m%s\x1b[0m "
    "\x1b[1;38;5;47m%s\x1b[0m "
    "\x1b[4m%s\x1b[0m "
    "\x1b[1;38;5;014m%d\x1b[0m "
    "<\x1b[1;38;5;226m%s\x1b[0m>\n",
    req_time, addr, cmd, params, nbytes, content
  );
}

char **str_split(char *a_str, const char a_delim) {
  char** result    = 0;
  size_t count     = 0;
  char* tmp        = a_str;
  char* last_comma = 0;
  char delim[2];
  delim[0] = a_delim;
  delim[1] = 0;

  /* Count how many elements will be extracted. */
  while (*tmp) {
    if (a_delim == *tmp) {
      count++;
      last_comma = tmp;
    }
    tmp++;
  }

  /* Add space for trailing token. */
  count += last_comma < (a_str + strlen(a_str) - 1);

  /* Add space for terminating null string so caller
     knows where the list of returned strings ends. */
  count++;

  result = malloc(sizeof(char*) * count);

  if (result) {
    size_t idx  = 0;
    char* token = strtok(a_str, delim);

    while (token) {
      assert(idx < count);
      *(result + idx++) = strdup(token);
      token = strtok(0, delim);
    }
    assert(idx == count - 1);
    *(result + idx) = 0;
  }

  return result;
}

void print_arr(char *name, int *arr) {
  printf("%s: [ ", name);
  for(int i = 0; i < sizeof(*arr) / sizeof(arr[0]); i++)
    printf("%d ", arr[i]);
  printf("]\n");
}