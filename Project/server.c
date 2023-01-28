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
Request req;
Response res;
int receiver[MAX_SPECTATOR + 2];
int send_type;

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

void disconnect(ClientAddr clnt_addr, PlayerTree *playertree, Request *req, Response *res) {
  int player_id, client_fd;

  if(sscanf(req->header.params, "sock=%d&player_id=%d", &client_fd, &player_id) != 2) {
    responsify(res, 400, NULL, "Bad request. Usage: EXIT /exit sock=...&player_id=...", SEND_ME);
    return;
  }

  Player *player_found = player_find(playertree, player_id);
  player_found->sock = 0;
  time_print(clnt_addr.address, "OFFLINE", "", "", 0);
  close(client_fd);
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

void receiver_build(GameTree *gametree, PlayerTree *playertree, int curr_fd) {
  int player_id = 0, game_id = 0, i = 0;

  // TODO: Filter to get game id and player id
  char *game_id_str = "game_id";
  char *player_id_str = "player_id";
  char **params = str_split(req.header.params, '&');
  while(params[i]) {
    if(strstr(params[i], game_id_str) != NULL)
      sscanf(params[i], "game_id=%d", &game_id);
    if(strstr(params[i], player_id_str) != NULL)
      sscanf(params[i], "player_id=%d", &player_id);
    i++;
  }

  receiver[0] = curr_fd;

  switch (send_type) {
    case SEND_ME:
      break;

    case SEND_JOINER: {
      Game *game_found = game_find(gametree, game_id);
      // TODO: Send to all spectators and remain player
      int remain_player_id = (game_found->player1_id == player_id ? player_id : game_found->player2_id);
      // If there is a 2nd player -> send
      if(remain_player_id) receiver[1] = player_fd(playertree, remain_player_id);

      for(i = 0; i < MAX_SPECTATOR; i++) {
        if(game_found->spectators[i])
          receiver[i + 2] = player_fd(playertree, game_found->spectators[i]);
      }
    }

    case SEND_ALL:
      break;
  }
}

void route_handler(MYSQL *conn, ClientAddr clnt_addr, GameTree *gametree, PlayerTree *playertree) {
  char path[PATH_L], cmd[CMD_L];
  strcpy(cmd, req.header.command);
  strcpy(path, req.header.path);

  if (strcmp(cmd, "PLAY") == 0) {
    if(route(path, "/game/play")) game_handler(conn, gametree, playertree, &req, &res);
    if(route(path, "/game/create")) game_create(gametree, &req, &res);
    if(route(path, "/game/join")) game_join(gametree, &req, &res);
    if(route(path, "/game/quit")) game_quit(gametree, &req, &res);
  }

  else if (strcmp(cmd, "AUTH") == 0) {
    if(route(path, "/account/login")) signin(conn, clnt_addr, playertree, &req, &res);
    if(route(path, "/account/register")) signup(conn, playertree, &req, &res);
//    if(route(path, "/account/logout")) signout(conn, playertree, &req, &res);
  }

  else if (strcmp(cmd, "GET") == 0) {
    if(route(path, "/rank")) rank(conn, &req, &res);
    if(route(path, "/profile")) profile(conn, &req, &res);
    if(route(path, "/game/view")) game_view(gametree, &req, &res);
  }

  else if(strcmp(cmd, "CHAT") == 0) {
    if(route(path, "/chat")) chat(gametree, playertree, &req, &res);
  }

  else if(strcmp(cmd, "UPDATE") == 0) {
//    if(route(path, "/account/forgotPassword")) forgot_password(conn, &req, &res);
    if(route(path, "/account/updatePassword")) change_password(conn, playertree, &req, &res);
  }
  else if(strcmp(cmd, "EXIT") == 0) {
    if(route(path, "/exit")) disconnect(clnt_addr, playertree, &req, &res);
  }
  else {
    responsify(&res, 404, NULL, "Resource does not exist", SEND_ME);
  }
}

void handle_client(MYSQL *conn, ClientAddr clnt_addr, GameTree *gametree, PlayerTree *playertree) {
  int nbytes;
  while(1) {
    cleanup(&req, &res, receiver);
    if ((nbytes = get_req(clnt_addr.sock, &req)) <= 0) break;
    time_print(clnt_addr.address, req.header.command, req.header.path, req.header.params, nbytes);
    route_handler(conn, clnt_addr, gametree, playertree);
    send_type = res.send_type;
    receiver_build(gametree, playertree, clnt_addr.sock);
    send_res(receiver, res);
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
    time_print(client_addr.address, "ONLINE", "", "", 0);

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
