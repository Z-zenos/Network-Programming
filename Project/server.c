#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <mysql/mysql.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>

#include "auth.h"
#include "http.h"
#include "config.h"
#include "utils.h"
#include "notify.h"
#include "log.h"
#include "game.h"
#include "algo.h"
#include "player.h"

/* Global variable */
int server_fd;
Request req;
Response res;

void connect_database(MYSQL *conn) {
  if(mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASSWRD, DB_NAME, 0, NULL, 0) == NULL) {
    notify("error", N_DATABASE_CONNECT_FAILED);
    mysql_close(conn);
    exit(FAILURE);
  }
  logger("Connect database successfully...");
}

int route(char *path, char *route_name) {
  return str_start_with(path, route_name);
}

void route_handler(MYSQL *conn, GameTree *gametree, PlayerTree *playertree) {
  char path[PATH_L], cmd[CMD_L];
  strcpy(cmd, req.header.command);
  strcpy(path, req.header.path);

  if (strcmp(cmd, "PLAY") == 0) {
    if(route(path, "/game")) game_handler(gametree, playertree, &req, &res);
    if(route(path, "/createGame")) game_create(conn, gametree, playertree, &req, &res);
//    if(route(path, "/joinGame")) join_game(conn, &res, &res);
  }

  if (strcmp(cmd, "AUTH") == 0) {
    if(route(path, "/account/login")) signin(conn, &req, &res);
    if(route(path, "/account/register")) signup(conn, &req, &res);
  }

  if (strcmp(cmd, "GET") == 0) {
    if(route(path, "/rank")) rank(conn, &req, &res);
//    if(route(path, "/profile")) profile(conn, &req, &res);
//    if(route(path, "/viewgame")) view_game(conn, &req, &res);
  }

  if(strcmp(cmd, "CHAT") == 0) {

  }

  if(strcmp(cmd, "UPDATE")) {
//    if(route(path, "/account/forgotPassword")) forgot_password(conn, &req, &res);
//    if(route(path, "/account/updatePassword")) change_password(conn, &req, &res);
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

void handle_client(MYSQL *conn, GameTree *gametree, PlayerTree *playertree, ClientAddr client_addr) {
  char cmd[CMD_L], reqStr[REQ_L], resStr[RES_L];
  while(1) {
    h_clear(cmd, reqStr, resStr);
    if (get_req(client_addr.sock, &req) == FAILURE) break;
    route_handler(conn, gametree, playertree);
    send_res(client_addr.sock, res);
  }
}

// Structure of arguments to pass to client thread
typedef struct ThreadArgs {
  ClientAddr client_addr; // Socket descriptor for client
  MYSQL *conn;
  GameTree *gametree;
  PlayerTree *playertree;
} ThreadArgs;

// Each thread executes this function
void *ThreadMain(void *threadArgs) {
  // Guarantees that thread resources are deallocated upon return
  pthread_detach(pthread_self());

  // Extract socket file descriptor argument
  ClientAddr client_addr = ((ThreadArgs *)threadArgs)->client_addr;
  MYSQL *conn = ((ThreadArgs *)threadArgs)->conn;
  GameTree *gametree = ((ThreadArgs *)threadArgs)->gametree;
  PlayerTree *playertree = ((ThreadArgs *)threadArgs)->playertree;
  free(threadArgs); // Deallocate memory for argument

  handle_client(conn, gametree, playertree, client_addr);
  close(client_addr.sock);
  return(NULL);
}

void server_listen(MYSQL *conn, GameTree *gametree, PlayerTree *playertree) {
  for(;;) {
    ClientAddr client_addr = accept_conn(server_fd);

    // Create separate memory for client argument
    ThreadArgs *threadArgs = (ThreadArgs *) malloc(sizeof (ThreadArgs));
    if(threadArgs == NULL) {
      log_error("malloc() failed");
      close(server_fd);
      exit(FAILURE);
    }

    threadArgs->client_addr = client_addr;
    threadArgs->conn = conn;
    threadArgs->gametree = gametree;
    threadArgs->playertree = playertree;

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
  srand(time(NULL));   // Initialization, should only be called once.
  handle_signal();

  MYSQL *conn = mysql_init(NULL);
  GameTree *gametree;
  PlayerTree *playertree;

  if(conn == NULL) {
    log_error("%s", mysql_error(conn));
    exit(FAILURE);
  }

  log("success", "Build app successfully...");

  connect_database(conn);
  gametree = game_new();
  playertree = player_build(conn);

  server_fd = server_init(argv[1]);
  server_listen(conn, gametree, playertree);

  game_drop(gametree);
  mysql_close(conn);
  return SUCCESS;
}
