#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/socket.h>
#include <mysql/mysql.h>
#include <pthread.h>
#include <signal.h>

#include "auth.h"
#include "http.h"
#include "config.h"
#include "utils.h"
#include "notify.h"
#include "log.h"
#include "game.h"
#include "algo.h"

int server_fd;

void connect_database(MYSQL *conn) {
  if(mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASSWRD, DB_NAME, 0, NULL, 0) == NULL) {
    notify("error", N_DATABASE_CONNECT_FAILED);
    log_error("%s", mysql_error(conn));
    mysql_close(conn);
    exit(FAILURE);
  }
}

int route(char *req, char *route_name) {
  return str_start_with(req, route_name);
}

void route_handler(MYSQL *conn, GameTree * gametree, Message msg, char *res) {
  char path[PATH_L], cmd[CMD_L];
  strcpy(cmd, msg.header.command);
  strcpy(path, msg.header.path);

  if (strcmp(cmd, "PLAY") == 0) {
//    if(route(path, "/game")) game_handler(gametree, msg, res);
//    if(route(path, "/createGame")) create_game(conn, msg);
//    if(route(path, "/joinGame")) join_game(conn, msg);
  }

  if (strcmp(cmd, "AUTH") == 0) {
    if(route(path, "/account/login")) signin(conn, msg, res);
    if(route(path, "/account/register")) signup(conn, msg, res);
  }

  if (strcmp(cmd, "GET") == 0) {
//    if(route(path, "/rank")) rank(conn, msg);
//    if(route(path, "/profile")) profile(conn, msg);
//    if(route(path, "/viewgame")) view_game(conn, msg);
  }

  if(strcmp(cmd, "CHAT") == 0) {

  }

  if(strcmp(cmd, "UPDATE")) {
//    if(route(path, "/account/forgotPassword")) forgot_password(conn, msg);
//    if(route(path, "/account/updatePassword")) change_password(conn, msg);
  }
}

void signalHandler(int signo) {
  switch (signo) {
    case SIGINT:
      log_warn("Caught signal Ctrl + C, coming out...\n");
      break;
    case SIGQUIT:
      log_warn("Caught signal Ctrl + \\, coming out...\n");
      break;
    case SIGHUP:
      log_warn("The terminal with the program (or some other parent if relevant) dies, coming out...\n");
      break;
    case SIGTERM:
      log_warn("The termination request (sent by the kill program by default and other similar tools), coming out...\n");
      break;
    case SIGUSR1:
      log_warn("Killing the program, coming out...\n");
      break;
  }

  close(server_fd);
  exit(SUCCESS);
}

void handle_signal() {
  signal(SIGINT, signalHandler);
  signal(SIGQUIT, signalHandler);
  signal(SIGHUP, signalHandler);
  signal(SIGTERM, signalHandler);
  signal(SIGUSR1, signalHandler);
}

void handle_client(MYSQL *conn, GameTree *gametree, Client client) {
  char cmd[CMD_L], req[REQ_L], res[RES_L];
  Message msg;
  while(1) {
    clear(cmd, req, res);
    if (get_req(client.sock, req) == FAILURE) break;
    m_parse(&msg, req);
    route_handler(conn, gametree, msg, res);
    send_res(client.sock, res);
  }
}

// Structure of arguments to pass to client thread
typedef struct ThreadArgs {
  Client client; // Socket descriptor for client
  MYSQL *conn;
  GameTree *gametree;
} ThreadArgs;

// Each thread executes this function
void *ThreadMain(void *threadArgs) {
  // Guarantees that thread resources are deallocated upon return
  pthread_detach(pthread_self());

  // Extract socket file descriptor argument
  Client client = ((ThreadArgs *)threadArgs)->client;
  MYSQL *conn = ((ThreadArgs *)threadArgs)->conn;
  GameTree *gametree = ((ThreadArgs *)threadArgs)->gametree;
  free(threadArgs); // Deallocate memory for argument

  handle_client(conn, gametree, client);
  close(client.sock);
  return(NULL);
}

void server_listen(MYSQL *conn, GameTree *gametree) {
  for(;;) {
    Client client = accept_conn(server_fd);

    // Create separate memory for client argument
    ThreadArgs *threadArgs = (ThreadArgs *) malloc(sizeof (ThreadArgs));
    if(threadArgs == NULL) {
      log_error("malloc() failed");
      close(server_fd);
      exit(FAILURE);
    }

    threadArgs->client = client;
    threadArgs->conn = conn;
    threadArgs->gametree = gametree;

    // Create client thread
    pthread_t threadID;
    int rtnVal = pthread_create(&threadID, NULL, ThreadMain, threadArgs);
    if(rtnVal != 0) {
      log_error("pthread_create() failed with thread %lu\n", (unsigned long int)threadID);
      close(server_fd);
      exit(FAILURE);
    }
  }
}

int main(int argc, char *argv[]) {
  handle_signal();

  server_fd = server_init(argv[1]);

  MYSQL *conn = mysql_init(NULL);
  GameTree *gametree;

  if(conn == NULL) {
    log_error("%s", mysql_error(conn));
    exit(FAILURE);
  }

  connect_database(conn);
  gametree = game_new();


//  signup(conn);
//  signin(conn);

  server_listen(conn, gametree);
  mysql_close(conn);
  return SUCCESS;
}
