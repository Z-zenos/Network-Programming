#include <mysql/mysql.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "auth.h"
#include "config.h"
#include "game.h"
#include "http.h"
#include "player.h"
#include "utils.h"

/* Global variable */
int server_fd;
Request req;
Response res;

void signalHandler(int signo) {
  switch (signo) {
    case SIGINT:
      logger(L_WARN, 1, "Caught signal Ctrl + C, coming out...\n");
      break;
    case SIGQUIT:
      logger(L_WARN, 1, "Caught signal Ctrl + \\, coming out...\n");
      break;
    case SIGHUP:
      logger(L_WARN, 1, "The terminal with the program (or some other parent if relevant) dies, coming out...\n");
      break;
    case SIGTERM:
      logger(L_WARN, 1, "The termination request (sent by the kill program by default and other similar tools), coming out...\n");
      break;
    case SIGUSR1:
      logger(L_WARN, 1, "Killing the program, coming out...\n");
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

void connect_database(MYSQL *conn) {
  if(mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASSWRD, DB_NAME, 0, NULL, 0) == NULL) {
    logger(L_ERROR, 1, "Connect to database failed !");
    mysql_close(conn);
    exit(FAILURE);
  }
  logger(L_SUCCESS, 1, "Connect database successfully...");
}

int route(char *path, char *route_name) { return str_start_with(path, route_name); }

void route_handler(MYSQL *conn, GameTree *gametree, PlayerTree *playertree) {
  char path[PATH_L], cmd[CMD_L];
  strcpy(cmd, req.header.command);
  strcpy(path, req.header.path);

  if (strcmp(cmd, "PLAY") == 0) {
    if(route(path, "/game")) game_handler(gametree, playertree, &req, &res);
    if(route(path, "/createGame")) game_create(conn, gametree, &req, &res);
//    if(route(path, "/joinGame")) join_game(conn, &res, &res);
  }

  if (strcmp(cmd, "AUTH") == 0) {
    if(route(path, "/account/login")) signin(conn, &req, &res);
    if(route(path, "/account/register")) signup(conn, &req, &res);
  }

  if (strcmp(cmd, "GET") == 0) {
    if(route(path, "/rank")) rank(conn, &req, &res);
//    if(route(path, "/profile")) profile(conn, &req, &res);
    if(route(path, "/viewgame")) game_view(conn, gametree, &req, &res);
  }

  if(strcmp(cmd, "CHAT") == 0) {

  }

  if(strcmp(cmd, "UPDATE") == 0) {
//    if(route(path, "/account/forgotPassword")) forgot_password(conn, &req, &res);
//    if(route(path, "/account/updatePassword")) change_password(conn, &req, &res);
  }
}

void handle_client(MYSQL *conn, GameTree *gametree, PlayerTree *playertree, ClientAddr client_addr) {
  int nbytes;
  while(1) {
    cleanup(&req, &res);
    if ((nbytes = get_req(client_addr.sock, &req)) <= 0) break;
    time_print(client_addr.address, req.header.command, req.header.path, nbytes);
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
    time_print(client_addr.address, "ACCESS", "/play", 1);

    // Create separate memory for client argument
    ThreadArgs *threadArgs = (ThreadArgs *) malloc(sizeof (ThreadArgs));
    if(threadArgs == NULL) {
      logger(L_ERROR, 1, "malloc() failed");
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
      close(server_fd);
      exit(FAILURE);
    }
  }
}

int main(int argc, char *argv[]) {
  srand(time(NULL));
  handle_signal();

  MYSQL *conn = mysql_init(NULL);
  GameTree *gametree;
  PlayerTree *playertree;

  if(conn == NULL) {
    logger(L_ERROR, 1, mysql_error(conn));
    exit(FAILURE);
  }

  connect_database(conn);
  gametree = game_new();
  Game g = {
    .id = 1,
    .views = 199,
    .num_move = 48,
    .result = 0,
    .turn = 'O',
    .player1_id = 1,
    .player2_id = 2,
    .board = {
      {'_', 'O', 'X'},
      {'X', '_', '_'},
      {'O', 'X', '_'}
    } ,
    .col = 1,
    .row = 0,
  };

  game_add(gametree, g);
  playertree = player_build(conn);

  server_fd = server_init(argv[1]);
  server_listen(conn, gametree, playertree);

  game_drop(gametree);
  mysql_close(conn);
  return SUCCESS;
}
