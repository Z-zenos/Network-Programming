#include <string.h>
#include <termios.h>
#include <stdlib.h>
#include <ctype.h>

#include "utils.h"
#include "message.h"
#include "env.h"

void clear_buffer() {
  int c;
  while ((c = getchar()) != '\n' && c != EOF);
}

bool str_start_with(char *str, char *pre) {
  return strncmp(pre, str, strlen(pre)) == 0;
}

ssize_t getpasswd(char **pw, size_t sz, int mask, FILE *fp) {
  if (!pw || !sz || !fp) return -1;       /* validate input   */

  #ifdef MAX_PASSWORD
    if (sz > MAX_PASSWORD) sz = MAX_PASSWORD;
  #endif

  if (*pw == NULL) {              /* reallocate if no address */
    void *tmp = realloc (*pw, sz * sizeof **pw);
    if (!tmp)
      return -1;
    memset (tmp, 0, sz);    /* initialize memory to 0   */
    *pw =  (char*) tmp;
  }

  size_t idx = 0;         /* index, number of chars in read   */
  int c = 0;

  struct termios old_kbd_mode;    /* orig keyboard settings   */
  struct termios new_kbd_mode;

  if (tcgetattr (0, &old_kbd_mode)) { /* save orig settings   */
    fprintf (stderr, "%s() error: tcgetattr failed.\n", __func__);
    return -1;
  }   /* copy old to new */
  memcpy (&new_kbd_mode, &old_kbd_mode, sizeof(struct termios));

  new_kbd_mode.c_lflag &= ~(ICANON | ECHO);  /* new kbd flags */
  new_kbd_mode.c_cc[VTIME] = 0;
  new_kbd_mode.c_cc[VMIN] = 1;
  if (tcsetattr (0, TCSANOW, &new_kbd_mode)) {
    fprintf (stderr, "%s() error: tcsetattr failed.\n", __func__);
    return -1;
  }

  /* read chars from fp, mask if valid char specified */
  while (((c = fgetc (fp)) != '\n' && c != EOF && idx < sz - 1) || (idx == sz - 1 && c == 127)) {
    if (c != 127) {
      if (31 < mask && mask < 127)    /* valid ascii char */
        fputc (mask, stdout);
      (*pw)[idx++] = c;
    }
    else if (idx > 0) {         /* handle backspace (del)   */
      if (31 < mask && mask < 127) {
        fputc (0x8, stdout);
        fputc (' ', stdout);
        fputc (0x8, stdout);
      }
      (*pw)[--idx] = 0;
    }
  }

  (*pw)[idx] = 0; /* null-terminate   */

  /* reset original keyboard  */
  if (tcsetattr (0, TCSANOW, &old_kbd_mode)) {
    fprintf (stderr, "%s() error: tcsetattr failed.\n", __func__);
    return -1;
  }

  if (idx == sz - 1 && c != '\n') /* warn if pw truncated */
    fprintf (stderr, " (%s() warning: truncated at %zu chars.)\n", __func__, sz - 1);

  return idx; /* number of chars in passwd    */
}

bool str_has_space(char *str) {
  // Check space in input
  int space;
  for(space = 0; space < strlen(str); space++)
    if(str[space] == ' ' || str[space] == '\t') {
      return true;
    }

  return false;
}

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

bool is_number(char *str) {
  while (*str) {
    if (isdigit(*str++) == 0) return false;
  }

  return true;
}

bool isChar(char c) {
  return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'));
}

bool isDigit(const char c) {
  return (c >= '0' && c <= '9');
}

bool is_valid_email(char *email) {
  if (!isChar(email[0])) return FAILURE;

  int At = -1, Dot = -1;
  
  for (int i = 0; i < strlen(email); i++) {
    if (email[i] == '@') At = i;
    else if (email[i] == '.') Dot = i;
  }
  
  if (At == -1 || Dot == -1) return FAILURE;
  if (At > Dot) return FAILURE;
  
  return Dot < (strlen(email) - 1);
}

bool is_valid_text(char *str) {
  while (*str) {
    if (isalnum(*str++) == 0) return false;
  }

  return true;
}

int input(char *type, char *str, int max_length) {
  if(!type) return FAILURE;
  char input_console[MAX_LENGTH_INPUT];

  if(strcmp(type, "password") == 0) {
    char *pw = input_console;
    getpasswd (&pw, MAX_LENGTH_PASSWORD, '*', stdin);
    printf("\n");
  }
  else {
    fgets(input_console, MAX_LENGTH_INPUT, stdin);
    input_console[strlen(input_console) - 1] = '\0';
  }

  if(strlen(input_console) <= 0) {
    t3_message("error", T3_INPUT_EMPTY);
    return FAILURE;
  }

  if(strlen(input_console) > max_length) {
    t3_message("error", T3_INPUT_TOO_LONG);
    return FAILURE;
  }

  if(str_has_space(input_console)) {
    t3_message("error", T3_INPUT_INVALID);
    return FAILURE;
  }

  strcpy(input_console, str_trim(input_console));

  if(strcmp(type, "text") == 0 && !is_valid_text(input_console)) {
    t3_message("error", T3_INPUT_INVALID);
    return FAILURE;
  }
  else if(strcmp(type, "number") == 0 && !is_number(input_console)) {
    t3_message("error", T3_INPUT_INVALID);
    return FAILURE;
  }
  else if(strcmp(type, "email") == 0 && !is_valid_email(input_console)) {
    t3_message("error", T3_EMAIL_INVALID);
    return FAILURE;
  }

  sscanf(input_console, "%[^\n]s", str);
  return SUCCESS;
}

int input_label(char *label, char *str, char *type,  int max_length) {
  char opt[MAX_LENGTH_INPUT];

  do {
    strcpy(str, "");
    strcpy(opt, "");

    printf("%s: ", label);

    if (input(type, str, max_length)) {
      return SUCCESS;
    }

    do {
      printf("Would you like to continue? (y/n): ");
      input("text", opt, MAX_LENGTH_INPUT);
      if(strlen(opt) > 1)
        continue;
    } while(!(opt[0] == 'y' || opt[0] == 'n'));
    if(opt[0] == 'y') continue;
    else if(opt[0] == 'n') break;
  } while(true);

  return FAILURE;
}

void str_clear(char *str) {
  strcpy(str, "");
}


