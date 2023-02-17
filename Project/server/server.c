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
BadWordsStorage *bad_word_storage;
int receiver[MAX_CLIENT];
int client_fds[MAX_CLIENT];
int number_clients = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

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
    case SIGSEGV:
      logger(L_WARN, "Attempting to access memory that doesn't belong to server program, coming out...\n");
      break;
  }

  for(int i = 0; i < MAX_CLIENT; i++)
    if(client_fds[i]) close(client_fds[i]);
  number_clients = 0;
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
  signal(SIGSEGV, signalHandler);
}

void connect_database(MYSQL *conn) {
  if(mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASSWRD, DB_NAME, 0, NULL, 0) == NULL) {
    logger(L_ERROR, "Connect to database failed !");
    mysql_close(conn);
    exit(FAILURE);
  }
  logger(L_SUCCESS, "Connect database successfully...");
}

int disconnect(MYSQL *conn, ClientAddr clnt_addr, GameTree *gametree, PlayerTree *playertree, Message *msg) {
  int player_id = atoi(map_val(msg->params, "player_id"));

  Player *player_found = player_find(playertree, player_id);
  player_found->sock = 0;
  player_found->is_online = false;
  time_print(clnt_addr.address, "EXIT APP", "", 0, "");
  for (int i = 0; i < MAX_CLIENT; i++) {
    if (client_fds[i] == clnt_addr.sock) {
      client_fds[i] = 0;
      number_clients--;
    }
  }

  // TODO: Handle player quit app while playing game
  if(player_found->is_playing) {
    Game *game;
    rbtrav_t *rbtrav;
    rbtrav = rbtnew();
    game = rbtfirst(rbtrav, gametree);

    do {
      if (game->player1_id == player_id || game->player2_id == player_id) {
        int opponent_id = game->player1_id == player_id ? game->player2_id : game->player1_id;
        Player *opponent = player_find(playertree, opponent_id);
        char query[QUERY_L];
        memset(query, '\0', QUERY_L);

        game->result = player_id;
        ++player_found->game;
        ++player_found->achievement.loss;

        ++opponent->game;
        ++opponent->achievement.win;
        opponent->achievement.points += 3;

        // TODO: Update database
        sprintf(
          query,
          "UPDATE players SET game = %d, win = %d, points = %d WHERE id = %d",
          opponent->game, opponent->achievement.win, opponent->achievement.points, opponent_id
        );
        mysql_query(conn, query);

        sprintf(
          query,
          "UPDATE players SET game = %d, loss = %d WHERE id = %d",
          player_found->game, player_found->achievement.loss, player_id
        );
        mysql_query(conn, query);

        sprintf(
          query,
          "INSERT INTO histories (player1_id, player2_id, result, num_moves) VALUES (%d, %d, -1, %d)",
          player_id, opponent_id, game->num_move);
        mysql_query(conn, query);
      }
    } while ((game = rbtnext(rbtrav)) != NULL);
  }

  close(clnt_addr.sock);
  return SUCCESS;
}

void route_handler(
  MYSQL *conn, ClientAddr clnt_addr, GameTree *gametree,
  PlayerTree *playertree, int *receiver
) {
  char cmd[CMD_L];
  strcpy(cmd, msg.command);

  /* GAME */
  if (strcmp(cmd, "GAME_FINISH") == 0)      game_finish(conn, gametree, playertree, &msg, receiver);
  if (strcmp(cmd, "CARO") == 0)             caro(gametree, playertree, &msg, receiver);
  if (strcmp(cmd, "GAME_QUICK") == 0)       game_quick(conn, gametree, playertree, &msg, receiver);
  if (strcmp(cmd, "GAME_CREATE") == 0)      game_create(gametree, &msg);
  if (strcmp(cmd, "GAME_CANCEL") == 0)      game_cancel(gametree, &msg);
  if (strcmp(cmd, "GAME_JOIN") == 0)        game_join(conn, gametree, playertree, &msg, receiver);
  if (strcmp(cmd, "GAME_QUIT") == 0)        game_quit(conn, gametree, playertree, &msg, receiver);

  /* DUEL */
  if (strcmp(cmd, "DUEL_REQUEST") == 0)      duel_request(playertree, &msg, receiver);
  if (strcmp(cmd, "DUEL") == 0)              duel_handler(conn, gametree, playertree, &msg, receiver);

  /* DRAW REQUEST */
  if (strcmp(cmd, "DRAW_REQUEST") == 0)      draw_request(playertree, &msg, receiver);
  if (strcmp(cmd, "DRAW") == 0)              draw_handler(conn, gametree, playertree, &msg, receiver);

  /* FRIEND */
  if(strcmp(cmd, "FRIEND_CHECK") == 0)      friend_check(conn, &msg);
  if(strcmp(cmd, "FRIEND_LIST") == 0)       friend_list(conn, playertree, &msg);
  if(strcmp(cmd, "FRIEND_ADD") == 0)        friend_add(conn, playertree, &msg, receiver);
  if(strcmp(cmd, "FRIEND_ACCEPT") == 0)     friend_accept(conn, playertree, &msg);

  /* AUTH */
  if(strcmp(cmd, "LOGIN") == 0)             signin(conn, clnt_addr, playertree, &msg);
  if(strcmp(cmd, "LOGOUT") == 0)            signout(playertree, &msg);
  if(strcmp(cmd, "REGISTER") == 0)          signup(conn, clnt_addr, playertree, &msg);
  if(strcmp(cmd, "PASSWORD_UPDATE") == 0)   change_password(conn, playertree, &msg);

  /* GET */
  if(strcmp(cmd, "RANK") == 0)              rank(conn, &msg);
  if(strcmp(cmd, "PROFILE") == 0)           profile(conn, &msg);
  if(strcmp(cmd, "GAME_LIST") == 0)         game_list(gametree, &msg);

  /* CHAT */
  if(strcmp(cmd, "CHAT") == 0)              chat(gametree, playertree, bad_word_storage, &msg, receiver);

  /* EXIT APP */
  if(strcmp(cmd, "EXIT") == 0)              disconnect(conn, clnt_addr, gametree, playertree, &msg);
}

void handle_client(MYSQL *conn, ClientAddr clnt_addr, GameTree *gametree, PlayerTree *playertree) {
  int nbytes;
  while(1) {
    if ((nbytes = get_msg(clnt_addr.sock, &msg)) <= 0) {
      time_print(clnt_addr.address, "EXIT APP", "", 0, "");
      for (int i = 0; i < MAX_CLIENT; i++) {
        if (client_fds[i] == clnt_addr.sock) {
          client_fds[i] = 0;
          number_clients--;
        }
      }
      close(clnt_addr.sock);
      break;
    }

    // LOCK resource
    pthread_mutex_lock(&mutex);

    time_print(clnt_addr.address, msg.command, msg.__params__, nbytes, msg.content);

    /*
     * receiver[0] no change -> send me
     * receiver[0] = -1 -> send all
     * receiver[0] changed after route handler complete -> send to one other
     * receiver[1] != 0 send in game for opponent and spectators
     * */
    receiver[0] = clnt_addr.sock;

    route_handler(conn, clnt_addr, gametree, playertree, receiver);
    send_msg(receiver[0] == -1 ? client_fds : receiver, msg);
    cleanup(&msg, receiver);

    // UNLOCK resource
    pthread_mutex_unlock(&mutex);
  }
}

typedef struct ThreadArgs {
  ClientAddr client_addr;
  MYSQL *conn;
  GameTree *gametree;
  PlayerTree *playertree;
} ThreadArgs;

void *ThreadMain(void *threadArgs) {
  // Guarantees that thread resources are deallocated upon return
  pthread_detach(pthread_self());

  // Extract argument from main thread
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

    // TODO: Reject client if numbers of client greater than 20
    if(number_clients == MAX_CLIENT) {
      pthread_mutex_lock(&mutex);
      receiver[0] = client_addr.sock;
      server_error(&msg);
      send_msg(receiver, msg);
      cleanup(&msg, receiver);
      pthread_mutex_unlock(&mutex);
      continue;
    }

    time_print(client_addr.address, "ONLINE", "", 0, "");
    client_fds[number_clients++] = client_addr.sock;

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
      server_error(&msg);
      close(server_fd);
      exit(FAILURE);
    }
  }
}

int main(int argc, char *argv[]) {
  if(argc != 2) {
    logger(L_INFO, "Usage: ./server <port>");
    return FAILURE;
  }

  srand(time(NULL));
  handle_signal();

  MYSQL *conn = mysql_init(NULL);
  bad_word_storage = bad_words_build();

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
    .num_move = 48,
    .result = -1,
    .player1_id = 2,
    .player2_id = 0,
    .password = ""
  };

  game_add(gametree, g);
  playertree = player_build(conn);

  server_fd = server_init(argv[1]);
  cleanup(&msg, receiver);

  server_listen(conn, gametree, playertree);

  game_drop(gametree);
  player_drop(playertree);
  bad_words_drop(bad_word_storage);
  if(msg.params) map_drop(msg.params);
  mysql_close(conn);
  close(server_fd);
  return SUCCESS;
}