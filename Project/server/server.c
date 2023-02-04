#include <mysql/mysql.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "auth.h"
#include "chat.h"
#include "config.h"
#include "game.h"
#include "http.h"
#include "player.h"
#include "utils.h"

/* Global variable */
int server_fd;
Message msg;
int receiver[MAX_CLIENT];
int client_fds[MAX_CLIENT];
int number_clients = 0;

void signalHandler(int signo) {
  switch (signo) {
    case SIGINT:
      logger(L_WARN, "Caught signal Ctrl + C, coming out...\n");
      break;
    case SIGQUIT:
      logger(L_WARN, "Caught signal Ctrl + \\, coming out...\n");
      break;
    case SIGHUP:
      logger(L_WARN, "The terminal with the program (or some other parent if relevant) dies, coming out...\n");
      break;
    case SIGTERM:
      logger(L_WARN, "The termination request (sent by the kill program by default and other similar tools), coming out...\n");
      break;
    case SIGUSR1:
      logger(L_WARN, "Killing the program, coming out...\n");
      break;
    case SIGABRT:
      logger(L_WARN, "Detect an internal error or some seriously broken constraint, coming out...\n");
      break;
  }

  for(int i = 0; i < MAX_CLIENT; i++)
    if(client_fds[i]) close(client_fds[i]);
  close(server_fd);
  exit(SUCCESS);
}

void handle_signal() {
  signal(SIGINT, signalHandler);
  signal(SIGQUIT, signalHandler);
  signal(SIGHUP, signalHandler);
  signal(SIGTERM, signalHandler);
  signal(SIGUSR1, signalHandler);
  signal(SIGABRT, signalHandler);
}

int disconnect(MYSQL *conn, ClientAddr clnt_addr, GameTree *gametree, PlayerTree *playertree, Message *msg, int *receiver) {
  int player_id = atoi(map_val(msg->params, "player_id"));

  Player *player_found = player_find(playertree, player_id);
  player_found->sock = 0;
  player_found->is_online = false;
  time_print(clnt_addr.address, "OFFLINE", "", 0, "");
  for(int i = 0; i < MAX_CLIENT; i++)
    if(client_fds[i] == clnt_addr.sock)
      client_fds[i] = 0;
  close(clnt_addr.sock);
  return SUCCESS;
}

void connect_database(MYSQL *conn) {
  if(mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASSWRD, DB_NAME, 0, NULL, 0) == NULL) {
    logger(L_ERROR, "Connect to database failed !");
    mysql_close(conn);
    exit(FAILURE);
  }
  logger(L_SUCCESS, "Connect database successfully...");
}

int route_null(MYSQL *conn, ClientAddr clnt_addr, GameTree *gametree, PlayerTree *playertree, Message *msg, int *receiver) {
  responsify(msg, "resource_null", NULL);
  return FAILURE;
}

int (*route_handler(MYSQL *conn, ClientAddr clnt_addr, GameTree *gametree, PlayerTree *playertree))
    (MYSQL *, ClientAddr, GameTree *, PlayerTree *, Message *, int *) {
  char cmd[CMD_L];
  strcpy(cmd, msg.command);

  /* GAME */
  if (strcmp(cmd, "GAME_PLAY") == 0)        return game_handler;
  if (strcmp(cmd, "GAME_CREATE") == 0)      return game_create;
  if (strcmp(cmd, "GAME_CANCEL") == 0)      return game_cancel;
  if (strcmp(cmd, "GAME_JOIN") == 0)        return game_join;
  if (strcmp(cmd, "GAME_QUIT") == 0)        return game_quit;

  /* FRIEND */
  if(strcmp(cmd, "FRIEND_CHECK") == 0)      return friend_check;
  if(strcmp(cmd, "FRIEND_LIST") == 0)       return friend_list;

  /* AUTH */
  if(strcmp(cmd, "LOGIN") == 0)             return signin;
  if(strcmp(cmd, "REGISTER") == 0)          return signup;
  if(strcmp(cmd, "PASSWORD_UPDATE") == 0)   return change_password;

  /* GET */
  if(strcmp(cmd, "RANK") == 0)              return rank;
  if(strcmp(cmd, "PROFILE") == 0)           return profile;
  if(strcmp(cmd, "GAME_VIEW") == 0)         return game_view;
  if(strcmp(cmd, "GAME_LIST") == 0)         return game_list;

  /* CHAT */
  if(strcmp(cmd, "CHAT") == 0)              return chat;

  /* QUIT*/
  if(strcmp(cmd, "EXIT") == 0)              return disconnect;

  return route_null;
}

void handle_client(MYSQL *conn, ClientAddr clnt_addr, GameTree *gametree, PlayerTree *playertree) {
  int nbytes;
  while(1) {
    cleanup(&msg, receiver);
    if ((nbytes = get_msg(clnt_addr.sock, &msg)) <= 0) {
      time_print(clnt_addr.address, "OFFLINE", "", 0, "");
      break;
    }
    time_print(clnt_addr.address, msg.command, msg.__params__, nbytes, msg.content);

    /*
     * receiver[0] no change -> send me
     * receiver[0] = -1 -> send all
     * receiver[0] changed after route handler complete -> send to one other
     * receiver[1] != 0 send in game for opponent and spectators
     * */
    receiver[0] = clnt_addr.sock;

    route_handler(conn, clnt_addr, gametree, playertree)(conn, clnt_addr, gametree, playertree, &msg, receiver);
    send_msg(receiver[0] == -1 ? client_fds : receiver, msg);
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
  ClientAddr clnt_addr = ((ThreadArgs *)threadArgs)->client_addr;
  MYSQL *conn = ((ThreadArgs *)threadArgs)->conn;
  GameTree *gametree = ((ThreadArgs *)threadArgs)->gametree;
  PlayerTree *playertree = ((ThreadArgs *)threadArgs)->playertree;
  free(threadArgs); // Deallocate memory for argument

  handle_client(conn, clnt_addr, gametree, playertree);
  close(clnt_addr.sock);
  return(NULL);
}

void server_listen(MYSQL *conn, GameTree *gametree, PlayerTree *playertree) {
  for(;;) {
    ClientAddr client_addr = accept_conn(server_fd);
    time_print(client_addr.address, "ONLINE", "", 0, "");
    client_fds[number_clients++] = client_addr.sock;

    // Create separate memory for client argument
    ThreadArgs *threadArgs = (ThreadArgs *) malloc(sizeof (ThreadArgs));
    if(threadArgs == NULL) {
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

    /* Reduce CPU usage */
    sleep(1);
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
    logger(L_ERROR, mysql_error(conn));
    exit(FAILURE);
  }

  connect_database(conn);
  gametree = game_new();
  Game g = {
    .id = 1,
    .views = 9,
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
    .password = "abc"
  };
  memset(g.joiner, 0, sizeof g.joiner);

  game_add(gametree, g);
  playertree = player_build(conn);

  server_fd = server_init(argv[1]);
  server_listen(conn, gametree, playertree);

  game_drop(gametree);
  mysql_close(conn);
  return SUCCESS;
}
