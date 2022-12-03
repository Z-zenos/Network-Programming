#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>

#include "utils.h"
#include "env.h"
#include "message.h"

void finish_with_error(MYSQL *conn) {
  fprintf(stderr, "%s\n", mysql_error(conn));
  mysql_close(conn);
  exit(1);
}

int signin() {
  char username[MAX_LENGTH_USERNAME], password[MAX_LENGTH_PASSWORD];
  if(input_label("Username", username, "text", MAX_LENGTH_USERNAME)) {

  }

  input_label("Username", password, "password", MAX_LENGTH_USERNAME)
}


int signup();
int signout();
int change_password();
int forgot_password();