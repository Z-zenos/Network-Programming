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

int route_handler(MYSQL *conn, GameTree * gametree, Message msg, char *res) {
  if (strcmp(cmd, "PLAY") == 0) {
    if(route(req, "/game")) game_handler(gametree, msg, res);
  } else if (strcmp(cmd, "AUTH") == 0) {
    if(route(req, "/account/login")) signin(conn, msg);
    if(route(req, "/account/register")) signup(conn, msg);
//    if(route(req, "/account/changePassword"))        return createAccount;
  } else if (strcmp(cmd, "GET") == 0) {
    if(route(req, "/accounts/updatePassword"))  return updatePassword;
    if(route(req, "/accounts/logout"))          return logout;
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
