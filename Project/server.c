#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/socket.h>
#include <mysql/mysql.h>

#include "auth.h"
#include "env.h"
#include "utils.h"
#include "message.h"
#include "log.h"

void connect_database(MYSQL *conn) {
  if(mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASSWRD, DB_NAME, 0, NULL, 0) == NULL) {
    t3_message("error", T3_DATABASE_CONNECT_FAILED);
    log_error("%s", mysql_error(conn));
    mysql_close(conn);
    exit(FAILURE);
  }
}

int main(int argc, char *argv[]) {
  MYSQL *conn = mysql_init(NULL);
  if(conn == NULL) {
    log_error("%s", mysql_error(conn));
    exit(FAILURE);
  }

  connect_database(conn);

//  signup(conn);
  signin(conn);

  mysql_close(conn);
  return SUCCESS;
}
