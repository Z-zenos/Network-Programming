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

/*
 PLAY /game\r\n
 Content-Type: 0\r\n
 Params: game_id=1&player_id=1&turn=X\r\n
 \r\n
 * */
void handleGame(GameTree *gametree, char *req, char *res) {
  Message msg;
  m_parse(&msg, req);
  int game_id, player_id, col, row;
  char turn;

  // TODO: Get id
  sscanf(msg.header.params, "game_id=%d&player_id=%d&turn=%c&col=%d&row=%d", &game_id, &player_id, &turn, &col, &row);

  // TODO: Find game -> Update game board
  Game *game = game_find(gametree, game_id);
  game->turn = turn;
  game->num_move++;
  game->col = col;
  game->row = row;

  // TODO: Check state game
  if(checkWinning(game->board, turn, game->col, game->row)) {
    sprintf(res, "code: 200\r\ndata: win=%d", player_id);
    return;
  }

  sprintf(res, "code: 200\r\ndata: turn=%c&col=%d&row=%d", turn, col, row);
  return;
}



int route_null(char *request, char *response) { return FAIL; }

int route(char *req, char *route_name) {
  return str_start_with(req, route_name);
}

int (*routeHandler(char *method, char *req))(char *, char *) {
  if (strcmp(method, "GET") == 0) {
    if(route(req, "/accounts/verify/username")) return verifyUsername;
    if(route(req, "/accounts/verify/password")) return verifyPassword;
    if(route(req, "/accounts/ipv4"))            return getIPv4;
    if(route(req, "/accounts/domain"))          return getDomain;
    if(route(req, "/accounts/search"))          return getAccount;
  } else if (strcmp(method, "POST") == 0) {
    if(route(req, "/accounts/activate"))        return activateAccount;
    if(route(req, "/accounts/authen"))          return login;
    if(route(req, "/accounts/register"))        return createAccount;
  } else if (strcmp(method, "PATCH") == 0) {
    if(route(req, "/accounts/updatePassword"))  return updatePassword;
    if(route(req, "/accounts/logout"))          return logout;
  }
  return route_null;
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

void handleClient(Client client) {
  char method[MAX_METHOD_LENGTH], req[MAX_REQUEST_LENGTH], res[MAX_RESPONSE_LENGTH];

  while(1) {
    http_clear(method, req, res);
    if (get_request(client, method, req) == FAIL) break;
    routeHandler(method, req)(req, res);
    send_response(client.sock, res);
  }
}

// Structure of arguments to pass to client thread
typedef struct ThreadArgs {
  Client client; // Socket descriptor for client
} ThreadArgs;

// Each thread executes this function
void *ThreadMain(void *threadArgs) {
  // Guarantees that thread resources are deallocated upon return
  pthread_detach(pthread_self());

  // Extract socket file descriptor argument
  Client client = ((ThreadArgs *)threadArgs)->client;
  free(threadArgs); // Deallocate memory for argument

  handleClient(client);
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
      exit(FAIL);
    }

    threadArgs->client = client;

    // Create client thread
    pthread_t threadID;
    int rtnVal = pthread_create(&threadID, NULL, ThreadMain, threadArgs);
    if(rtnVal != 0) {
      log_error("pthread_create() failed with thread %lu\n", (unsigned long int)threadID);
      close(server_fd);
      exit(FAIL);
    }
  }
}

int main(int argc, char *argv[]) {
  handle_signal();

  server_fd = server_init(argv[2]);

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
