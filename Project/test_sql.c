#include <stdio.h>
#include <stdlib.h>
#include <mysql/mysql.h>

void finish_with_error(MYSQL *conn) {
  fprintf(stderr, "%s\n", mysql_error(conn));
  mysql_close(conn);
  exit(1);
}

int main(int argc, char **argv) {
  printf("MySQL client version: %s\n", mysql_get_client_info());
  //  exit(0);

  // mysql_init() function allocates or initialises a MYSQL object
  // suitable for mysql_real_connect() function
  MYSQL *conn = mysql_init(NULL);

  if(conn == NULL) {
    fprintf(stderr, "%s\n", mysql_error(conn));
    exit(1);
  }

  /*
    mysql_real_connect() function establishes a connection to the database.
    We provide connection handler, host name, username and password parameters
    to the function.
    The other four parameters are the database name, port number, unix socket and finally the client flag.
    We need superuser privileges to create a new database.
  */
  if(mysql_real_connect(conn, "localhost", "cuser", "@nhTu@n2oo1", "test_db_c", 0, NULL, 0) == NULL) {
    finish_with_error(conn);
  }

  //  Create new database
//  if(mysql_query(conn, "CREATE DATABASE test_db_c")) {
//    fprintf(stderr, "%s\n", mysql_error(conn));
//    mysql_close(conn);
//    exit(1);
//  }


  /* = CREATE TABLE AND INSERT DATA = */
//  if (mysql_query(conn, "DROP TABLE IF EXISTS cars")) {
//    finish_with_error(conn);
//  }
//
//  if (mysql_query(conn, "CREATE TABLE cars(id INT PRIMARY KEY AUTO_INCREMENT, name VARCHAR(255), price INT)")) {
//    finish_with_error(conn);
//  }
//
//  if (mysql_query(conn, "INSERT INTO cars VALUES(1,'Audi',52642)")) {
//    finish_with_error(conn);
//  }
//
//  if (mysql_query(conn, "INSERT INTO cars VALUES(2,'Mercedes',57127)")) {
//    finish_with_error(conn);
//  }
//
//  if (mysql_query(conn, "INSERT INTO cars VALUES(3,'Skoda',9000)")) {
//    finish_with_error(conn);
//  }
//
//  if (mysql_query(conn, "INSERT INTO cars VALUES(4,'Volvo',29000)")) {
//    finish_with_error(conn);
//  }
//
//  if (mysql_query(conn, "INSERT INTO cars VALUES(5,'Bentley',350000)")) {
//    finish_with_error(conn);
//  }
//
//  if (mysql_query(conn, "INSERT INTO cars VALUES(6,'Citroen',21000)")) {
//    finish_with_error(conn);
//  }
//
//  if (mysql_query(conn, "INSERT INTO cars VALUES(7,'Hummer',41400)")) {
//    finish_with_error(conn);
//  }
//
//  if (mysql_query(conn, "INSERT INTO cars VALUES(8,'Volkswagen',21600)")) {
//    finish_with_error(conn);
//  }

  if(mysql_query(conn, "SELECT * FROM cars")) {
    finish_with_error(conn);
  }

  MYSQL_RES *result = mysql_store_result(conn);
  if(result == NULL) {
    finish_with_error(conn);
  }

  int num_fields = mysql_num_fields(result);

  MYSQL_ROW row;

  printf("%-15s %-15s %-15s\n", "ID", "Name", "Price");
  while((row = mysql_fetch_row(result))) {
    for(int i = 0; i < num_fields; i++) {
      printf("%-15s ", row[i] ? row[i] : "NULL");
    }
    printf("\n");
  }

  mysql_free_result(result);
  mysql_close(conn);
  exit(0);
}