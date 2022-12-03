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

static const unsigned char e2a[256] = {
  0, 1, 2, 3, 156, 9, 134, 127, 151, 141, 142, 11, 12, 13, 14, 15,
  16, 17, 18, 19, 157, 133, 8, 135, 24, 25, 146, 143, 28, 29, 30, 31,
  128, 129, 130, 131, 132, 10, 23, 27, 136, 137, 138, 139, 140, 5, 6, 7,
  144, 145, 22, 147, 148, 149, 150, 4, 152, 153, 154, 155, 20, 21, 158, 26,
  32, 160, 161, 162, 163, 164, 165, 166, 167, 168, 91, 46, 60, 40, 43, 33,
  38, 169, 170, 171, 172, 173, 174, 175, 176, 177, 93, 36, 42, 41, 59, 94,
  45, 47, 178, 179, 180, 181, 182, 183, 184, 185, 124, 44, 37, 95, 62, 63,
  186, 187, 188, 189, 190, 191, 192, 193, 194, 96, 58, 35, 64, 39, 61, 34,
  195, 97, 98, 99, 100, 101, 102, 103, 104, 105, 196, 197, 198, 199, 200, 201,
  202, 106, 107, 108, 109, 110, 111, 112, 113, 114, 203, 204, 205, 206, 207, 208,
  209, 126, 115, 116, 117, 118, 119, 120, 121, 122, 210, 211, 212, 213, 214, 215,
  216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231,
  123, 65, 66, 67, 68, 69, 70, 71, 72, 73, 232, 233, 234, 235, 236, 237,
  125, 74, 75, 76, 77, 78, 79, 80, 81, 82, 238, 239, 240, 241, 242, 243,
  92, 159, 83, 84, 85, 86, 87, 88, 89, 90, 244, 245, 246, 247, 248, 249,
  48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 250, 251, 252, 253, 254, 255
};

void ebcdicToAscii (unsigned char *s) {
  while (*s) {
    *s = e2a[(int) (*s)];
    s++;
  }
}

bool is_email(char *EM_Addr) {
  int count = 0;
  char conv_buf[MAX_LENGTH_EMAIL];
  char *c, *domain;
  char *special_chars = "()<>@,;:\"[]";

  /* The input is in EBCDIC so convert to ASCII first */
  strcpy(conv_buf, EM_Addr);
  ebcdicToAscii((unsigned char *)conv_buf);
  /* convert the special chars to ASCII */
  ebcdicToAscii((unsigned char *)special_chars);

  for(c = conv_buf; *c; c++) {
    /* if '"' and beginning or previous is a '.' or '"' */
    if (*c == 34 && (c == conv_buf || *(c - 1) == 46 || *(c - 1) == 34)) {
      while (*++c) {
        /* if '"' break, End of name */
        if (*c == 34)
          break;
        /* if '' and ' ' */
        if (*c == 92 && (*++c == 32))
          continue;
        /* if not between ' ' & '~' */
        if (*c <= 32 || *c > 127)
          return FAILURE;
      }
      /* if no more characters error */
      if (!*c++)
        return FAILURE;
      /* found '@' */
      if (*c == 64)
        break;
      /* '.' required */
      if (*c != 46)
        return FAILURE;
      continue;
    }
    if (*c == 64) {
      break;
    }
    /* make sure between ' ' && '~' */
    if (*c <= 32 || *c > 127) {
      return FAILURE;
    }
    /* check special chars */
    if (strchr(special_chars, *c)) {
      return FAILURE;
    }
  } /* end of for loop */

  /* found '@' */
  /* if at beginning or previous = '.' */
  if (c == conv_buf || *(c - 1) == 46)
    return FAILURE;

  /* next we validate the domain portion */
  /* if the next character is NULL */
  /* need domain ! */
  if (!*(domain = ++c))
    return FAILURE;
  do {
    /* if '.' */
    if (*c == 46) {
      /* if beginning or previous = '.' */
      if (c == domain || *(c - 1) == 46)
        return FAILURE;
      /* count '.' need at least 1 */
      count++;
    }
    /* make sure between ' ' and '~' */
    if (*c <= 32 || *c >= 127)
      return FAILURE;
    if (strchr(special_chars, *c))
      return FAILURE;
  } while (*++c); /* while valid char */
  return (count >= 1); /* return true if more than 1 '.' */
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
    t3_message(T3_INPUT_EMPTY);
    return FAILURE;
  }

  if(strlen(input_console) > max_length) {
    t3_message(T3_INPUT_TOO_LONG);
    return FAILURE;
  }

  if(str_has_space(input_console)) {
    t3_message(T3_INPUT_INVALID);
    return FAILURE;
  }

  strcpy(input_console, str_trim(input_console));

  if(strcmp(type, "text") == 0 && !is_valid_text(input_console)) {
    t3_message(T3_INPUT_INVALID);
    return FAILURE;
  }
  else if(strcmp(type, "number") == 0 && !is_number(input_console)) {
    t3_message(T3_INPUT_INVALID);
    return FAILURE;
  }
  else if(strcmp(type, "email") == 0 && !is_email(input_console)) {
    t3_message(T3_EMAIL_INVALID);
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


