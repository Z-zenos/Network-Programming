#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

#include "utils.h"
#include "constants.h"
#include "log.h"
#include "error.h"
#include "linkedlist.h"
#include "account.h"

char username[MAX_USERNAME];
int logged_in;
Account curr_user;

/**
 * @brief Clear user's input
 * 
 */
void clear_buffer() {
  int c;    
  while ((c = getchar()) != '\n' && c != EOF);
}

/**
 * @brief           Remove whitespace at begin and end of string.
 * @param string    Input string
 * @return          string trimmed
 * */
char *strtrim(char *str) {

  if( str == NULL ) { return NULL; }
  if( str[0] == '\0' ) { return str; }

  size_t len = 0;
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

/**
 * @brief Get user input from terminal
 * 
 * @param label             Label for input
 * @param str               Input string 
 * @param MAX_LENGTH_INPUT  Number characters allowed in input
 * @param isHide            Mode isHide for hide input. Ex: password
 */
void input(char *label, char *str, const int MAX_LENGTH_INPUT, bool isHide) {
  char user_input[BUFFER];

  do {
    printf("%s: ", label);

    if(isHide) {
      // If mode is hide then hide user input
      char *p = user_input;
      getpasswd (&p, MAX_PASSWORD, '*', stdin);
      printf("\n");
    }
    else {
      fgets(user_input, sizeof(user_input), stdin);
      user_input[strlen(user_input) - 1] = '\0';
    }

    // Check input if empty
    if (strlen(user_input) > 0) {
      if(strlen(user_input) > MAX_LENGTH_INPUT) {
        log_error("%s can only contain up to %d characters.\n", label, MAX_LENGTH_INPUT);
        continue;
      }

      strcpy(user_input, strtrim(user_input));
      if (sscanf(user_input, "%[^\n]s", str) == 1) {
        return;
      }
    }
    else {
      err_error(ERR_INPUT_EMPTY);
    }
      
  } while (1);
}

/**
 * @brief   getpasswd will read upto sz - 1 chars into pw, null-terminating 
 *          the resulting string. On success, the number of characters in 
 *          pw are returned, -1 otherwise.
 * @return  number of chars in passwd
 */

ssize_t getpasswd (char **pw, size_t sz, int mask, FILE *fp) {
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

void load_data(XOR_LL *ll) {
  FILE *fp = fopen(USER_FILE, "r");

  if(!fp) {
    log_error("Could not open file %s\n", USER_FILE);
    return;
  }

  rewind(fp);
  char line[BUFFER];
  Account *acc = NULL;

  while(fgets(line, BUFFER, fp)) {
    acc = malloc(sizeof(*acc));
    sscanf(line, "%s %s %d", acc->username, acc->password, &acc->status);
    xor_ll_push_tail(ll, acc, sizeof(acc));

    if(acc->status == -1) {
      logged_in = 1;
      _set_current_user_(*acc);
    }
  }

  if(strlen(curr_user.username)) {
    char name[MAX_USERNAME];
    strcpy(name, curr_user.username);
    log_info(strcat(name, ", have a nice day :)"));
  }
  else
    log_info("Have a nice day :)");
  fclose(fp);
}

void save_data(XOR_LL ll) {
  FILE *fp = fopen("account_updated.txt", "w");
  if(!fp) {
    err_error(ERR_OPEN_FILE);
    return;
  }
  rewind(fp);

  XOR_LL_ITERATOR itr = XOR_LL_ITERATOR_INITIALISER;
  XOR_LL_LOOP_HTT_RST(&ll, &itr) {
    Account *acc = (Account*)itr.node_data.ptr;
    fprintf(fp, "%s %s %d\n", acc->username, acc->password, acc->status);
  }

  remove("account.txt");
  rename("account_updated.txt", "account.txt");

  fclose(fp);
  return;
}

void loading() {

  long i;
  float progress = 0.0;
  int c = 0, x = 0, last_c = 0;
  /**
   * Print a basic template of the progress line.
  **/
  fprintf(stderr, "%3d%% [", (int)progress);
  for (x = 0; x < c; x++) {
    fprintf(stderr, "=");
  }
  for (x = c; x < WIDTH; x++) {
    fprintf(stderr, " ");
  }
  fprintf(stderr, "]");

  for(i = 1; i < MAX + 1; i++) {
    progress = i * 100.0 / MAX;
    c = (int) progress;
    /**
     * Update the template on each increment in progress.
    **/
    fprintf(stderr, "\n\033[F");
    fprintf(stderr, "%3d%%", (int)progress);
    fprintf(stderr, "\033[1C");
    fprintf(stderr, "\033[%dC\x1b[1;38;5;226m=\x1b[0m", last_c);
    for (x = last_c; x < c; x++) {
      fprintf(stderr, "=");
    }
    if(x < WIDTH) {
      fprintf(stderr, ">");
    }
    last_c = c;
  }
  /**
   * Write a finish line.
  **/
  printf("\n");
}
