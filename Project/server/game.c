#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "http.h"

#include "game.h"
#include "algo.h"
#include "player.h"
#include "utils.h"
#include "rbtree.h"

static int game_cmp(const void *p1, const void *p2) {
  Game *game1, *game2;

  game1 = (Game *)p1;
  game2 = (Game *)p2;

  if(game1->id > game2->id) return 1;
  else if(game1->id < game2->id) return -1;
  else return 0;
}

static void *game_dup(void *p) {
  void *dup_p;

  dup_p = calloc(1, sizeof(Game));
  memmove(dup_p, p, sizeof(Game));

  return dup_p;
}

static void game_rel(void *p) { free(p); }

GameTree *game_new() {
  rbtree_t *rbtree;
  rbtree = rbnew(game_cmp, game_dup, game_rel);
  logger(L_SUCCESS, "Build game tree successfully...");
  return rbtree;
}

void game_drop(GameTree *gametree) { rbdelete(gametree); }

int game_add(GameTree *gametree, Game new_game) {
  int ret;

  Game *game;
  game = calloc(1, sizeof(Game));
  game->id = new_game.id;
  game->views = new_game.views;
  game->turn = new_game.turn;
  game->result = new_game.result;
  game->player1_id = new_game.player1_id;
  game->player2_id = new_game.player2_id;
  game->chat_id = new_game.chat_id;
  game->num_move = new_game.num_move;
  game->col = new_game.col;
  game->row = new_game.row;
  strcpy(game->password, new_game.password);
  for ( int i = 0; i < BOARD_S; ++i ){
    memcpy(game->board[i], new_game.board[i], sizeof new_game.board[i]);
  }
  for ( int i = 0; i < MAX_SPECTATOR; ++i ){
    game->joiner[i] = new_game.joiner[i];
  }

  ret = rbinsert(gametree, (void *)game);
  if (ret == 0) {
    logger(L_ERROR, "Can't insert new game for players");
    free(game);
    return -1;
  }

  return 0;
}

int game_delete(GameTree *gametree, int id) {
  int ret;
  Game *game;

  game = calloc(1, sizeof(Game));
  game->id = id;

  ret = rberase(gametree, (void *)game);
  if (ret == 0) {
    logger(L_ERROR, "Can't delete game id: %d", id);
    free(game);
    return -1;
  }

  return 0;
}

Game *game_find(GameTree *gametree, int id) {
  Game *game, game_find;

  game_find.id = id;
  game = rbfind(gametree, &game_find);

  return !game ? NULL : game;
}

void game_print_board(char board[BOARD_S][BOARD_S]) {
  int i, j;
  for (i = 0; i < BOARD_S; i++) {
    for (j = 0; j < BOARD_S; j++) {
      printf("%c ", board[i][j]);
    }
    printf("\n");
  }
}

void game_info(GameTree *gametree) {
  Game *game;

  rbtrav_t *rbtrav;
  rbtrav = rbtnew();
  game = rbtfirst(rbtrav, gametree);
  printf("id %d - views: %d - turn: %c - num_moves: %d - result: %d\n", game->id, game->views, game->turn, game->num_move, game->result);
  game_print_board(game->board);


  while ((game = rbtnext(rbtrav)) != NULL) {
    printf("id %d - views: %d - turn: %c - num_moves: %d - result: %d\n", game->id, game->views, game->turn, game->num_move, game->result);
    game_print_board(game->board);
  }
}

/*
 PLAY /game\r\n
 Content-Type: 0\r\n
 Params: game_id=1&player_id=1&turn=X\r\n
 \r\n
 * */
void game_handler(MYSQL *conn, GameTree *gametree, PlayerTree *playertree, Request *req, Response *res) {
  int game_id, player_id, col, row, opponent_id;
  char turn;

  // TODO: Get id
  sscanf(req->header.params, "game_id=%d&player_id=%d&turn=%c&col=%d&row=%d", &game_id, &player_id, &turn, &col, &row);

  // TODO: Find game -> Update game board
  Game *game = game_find(gametree, game_id);
  game->turn = turn;
  game->num_move++;
  game->col = col;
  game->row = row;
  char dataStr[DATA_L];
  memset(dataStr, '\0', sizeof dataStr);

  // TODO: Check state game chưa ghi vào db này
  if(checkWinning(game->board, turn, game->col, game->row)) {
    Player *winner = player_find(playertree, player_id);
    opponent_id = game->player1_id == player_id ? game->player2_id : player_id;
    Player *losser = player_find(playertree, opponent_id);

    winner->achievement.win++;
    winner->achievement.points += 3;

    losser->achievement.loss++;
    losser->achievement.points -= 1;

    // TODO: Update data in database
    char query[QUERY_L];
    sprintf(query, "UPDATE players SET win = %d AND points = %d WHERE id = %d", winner->achievement.win, winner->achievement.points, winner->id);
    mysql_query(conn, query);
    sprintf(query, "UPDATE players SET win = %d AND points = %d WHERE id = %d", losser->achievement.win, losser->achievement.points, losser->id);
    mysql_query(conn, query);

    sprintf(dataStr, "win=%d", player_id);
    responsify(res, 200, NULL, dataStr, "Player id win", SEND_JOINER);
    return;
  }

  sprintf(dataStr, "turn=%c&col=%d&row=%d", turn, col, row);
  responsify(res, 200, NULL, dataStr, NULL, SEND_JOINER);
  return;
}

void game_create(ClientAddr clnt_addr, GameTree *gametree, PlayerTree *playertree, Request *req, Response *res) {
  int player_id;
  char game_pwd[PASSWORD_L];
  memset(game_pwd, '\0', PASSWORD_L);

  // TODO: Get player id
  if(sscanf(req->header.params, "player_id=%d&password=%s", &player_id, game_pwd) != 2) {
    responsify(res, 400, NULL, NULL, "Bad request: Usage: PLAY /game/create player_id=...&password=...", SEND_ME);
    return;
  }

  // TODO: random first turn for game board
  int r = rand() % 2;

  rbtrav_t *rbtrav;
  rbtrav = rbtnew();
  Game *last_game = rbtlast(rbtrav, gametree);

  // TODO: Create game
  Game new_game = {
    .id = !last_game ? 1 : last_game->id + 1,
    .views = 0,
    .num_move = 0,
    .result = 0,
    .turn = (r == 1) ? 'X' : 'O',
    .player1_id = player_id,
    .player2_id = 0,
    .col = 0,
    .row = 0,
  };

  strcpy(new_game.password, game_pwd);

  for ( int i = 0; i < BOARD_S; ++i ){
    memset(new_game.board[i], '_', sizeof new_game.board[i]);
  }
  memset(new_game.joiner, 0, sizeof new_game.joiner);
  new_game.joiner[0] = clnt_addr.sock;

  game_add(gametree, new_game);

  Player *player_found = player_find(playertree, player_id);
  player_found->sock = clnt_addr.sock;

  char dataStr[DATA_L];
  memset(dataStr, '\0', sizeof dataStr);
  sprintf(dataStr, "game_id=%d&password=%s&turn=%c", new_game.id, game_pwd, new_game.turn);
  responsify(res, 200, "game_created", dataStr, "Create new game successfully", SEND_ME);
  return;
}

char *game_board2string(char board[BOARD_S][BOARD_S]) {
  char *boardStr = (char*)calloc('\0', BOARD_S * BOARD_S);
  int i;
  for(i = 0; i < BOARD_S; i++)
    strcat(boardStr, board[i]);
  boardStr[BOARD_S * BOARD_S] = '\0';
  return boardStr;
}

void game_view(ClientAddr clnt_addr, GameTree *gametree, Request *req, Response *res) {
  int player_id = 0, game_id;
  char msgStr[MESSAGE_L], dataStr[DATA_L];
  memset(msgStr, '\0', MESSAGE_L);
  memset(dataStr, '\0', DATA_L);

  // TODO: Get player id if exists and game id from user
  if(sscanf(req->header.params, "game_id=%d&player_id=%d", &game_id, &player_id) <= 0) {
    responsify(res, 400, NULL, NULL, "Bad request. Usage: GET /viewgame game_id=...[&player_id=...]", SEND_ME);
    return;
  }

  // TODO: Find game room for player
  Game *game_found = game_find(gametree, game_id);

  if(!game_found) {
    sprintf(msgStr, "Game [%d] does not exist", game_id);
    responsify(res, 400, NULL, NULL, msgStr, SEND_ME);
    return;
  }

  ++game_found->views;
  if(game_found->views > MAX_SPECTATOR) {
    responsify(res, 403, NULL, NULL, "Max spectators reached", SEND_ME);
    return;
  }

  for(int i = 2; i < MAX_SPECTATOR + 2; i++) {
    if (game_found->joiner[i] == 0) {
      game_found->joiner[i] = clnt_addr.sock;
      break;
    }
  }

  sprintf(
    dataStr,
    "game_id=%d&turn=%d&views=%d&num_move=%d&player1_id=%d&player2_id=%d&board=[%s]&col=%d&row=%d",
    game_found->id, game_found->turn, game_found->views, game_found->num_move,
    game_found->player1_id, game_found->player2_id,
    game_board2string(game_found->board), game_found->col, game_found->row
  );

  sprintf(msgStr, "You have become a spectator of the game [%d]", game_id);
  responsify(res, 200, NULL, dataStr, msgStr, SEND_JOINER);
  return;
}

void game_join(ClientAddr clnt_addr, GameTree *gametree, PlayerTree *playertree, Request *req, Response *res) {
  int player_id, game_id;
  char msgStr[MESSAGE_L], dataStr[DATA_L];
  memset(msgStr, '\0', MESSAGE_L);
  memset(dataStr, '\0', DATA_L);


  // TODO: Get player id if exists and game id from user
  if(sscanf(req->header.params, "game_id=%d&player_id=%d", &game_id, &player_id) != 2) {
    responsify(res, 400, NULL, NULL, "Bad request. Usage: PLAY /game/join game_id=...&player_id=...", SEND_ME);
    return;
  }

  // TODO: AUTH player: If player haven't registered yet

  // TODO: Find game room for player
  Game *game_found = game_find(gametree, game_id);

  if(!game_found) {
    sprintf(msgStr, "Game [%d] does not exist", game_id);
    responsify(res, 400, NULL, NULL, msgStr, SEND_ME);
    return;
  }

  if(game_found->player1_id) game_found->player2_id = player_id;
  else game_found->player1_id = player_id;

  if(game_found->joiner[0]) game_found->joiner[1] = clnt_addr.sock;
  else game_found->joiner[0] = clnt_addr.sock;

  Player *player_found = player_find(playertree, player_id);
  player_found->sock = clnt_addr.sock;

  sprintf(
    dataStr,
    "game_id=%d&turn=%d&views=%d&num_move=%d&player1_id=%d&player2_id=%d&board=[%s]&col=%d&row=%d",
    game_found->id, game_found->turn, game_found->views, game_found->num_move,
    game_found->player1_id, game_found->player2_id,
    game_board2string(game_found->board), game_found->col, game_found->row
  );

  responsify(res, 200, "game_joined", dataStr, "Join game successfully", SEND_JOINER);
  return;
}

void game_quit(ClientAddr clnt_addr, GameTree *gametree, Request *req, Response *res) {
  int player_id, game_id;
  char msgStr[MESSAGE_L], dataStr[DATA_L];
  memset(msgStr, '\0', MESSAGE_L);
  memset(dataStr, '\0', DATA_L);

  // TODO: Get player id if exists and game id from user
  if(sscanf(req->header.params, "game_id=%d&player_id=%d", &game_id, &player_id) != 2) {
    responsify(res, 400, NULL, NULL, "Bad request. Usage: PLAY /game/quit game_id=...&player_id=...", SEND_ME);
    return;
  }

  // TODO: Find game room for player
  Game *game_found = game_find(gametree, game_id);

  if(!game_found) {
    sprintf(msgStr, "Game [%d] does not exist", game_id);
    responsify(res, 400, NULL, NULL, msgStr, SEND_ME);
    return;
  }

  // TODO: Update game state: Unset socket of player or spectator
  if(game_found->joiner[0] == clnt_addr.sock) game_found->joiner[0] = 0;
  else game_found->joiner[1] = 0;

  if(game_found->player1_id == player_id) game_found->player1_id = 0;
  else if(game_found->player2_id == player_id) game_found->player2_id = 0;
  else {
    game_found->views--;
    for(int i = 2; i < MAX_SPECTATOR + 2; i++) {
      if(game_found->joiner[i] == clnt_addr.sock) {
        game_found->joiner[i] = 0;
        break;
      }
    }
  }

  sprintf(
    dataStr,
    "game_id=%d&turn=%d&views=%d&num_move=%d&player1_id=%d&player2_id=%d&board=[%s]&col=%d&row=%d",
    game_found->id, game_found->turn, game_found->views, game_found->num_move,
    game_found->player1_id, game_found->player2_id,
    game_board2string(game_found->board), game_found->col, game_found->row
  );

  // TODO: Send response to quited player
  responsify(res, 200, "game_exited", dataStr, "Quit game successfully", SEND_JOINER);
  return;
}

void game_list(GameTree *gametree, Request *req, Response *res) {
  char dataStr[DATA_L];
  memset(dataStr, '\0', DATA_L);

  Game *game;

  rbtrav_t *rbtrav;
  rbtrav = rbtnew();
  game = rbtfirst(rbtrav, gametree);
  char line[1000];
  memset(line, '\0', 100);

  do {
    sprintf(
      line,
      "game_id=%d&password=%s&views=%d&num_move=%d&player1_id=%d&player2_id=%d;",
      game->id, game->password, game->views, game->num_move,
      game->player1_id, game->player2_id
    );
    strcat(dataStr, line);
  } while ((game = rbtnext(rbtrav)) != NULL);

  // TODO: Remove ; at last data
  dataStr[strlen(dataStr) - 1] = '\0';

  responsify(res, 200, "game_list", dataStr, "Get list of game successfully", SEND_ME);
  return;
}

/*
int main() {
  GameTree *gametree;
  gametree = game_new();
  Game g1 = { .id = 1, .views = 34, .num_move = 12, .result = 0, .turn = 'X', .board = {{'_', '_'}, {'X', '_'}} };
  Game g2 = { .id = 5, .views = 4, .num_move = 3, .result = 0, .turn = 'O', .board = {{'_', 'O'}, {'X', '_'}} };
  Game g3 = { .id = 3, .views = 199, .num_move = 48, .result = 0, .turn = 'X', .board = {{'_', '_'}, {'X', '_'}} };
  game_add(gametree, g1);
  game_add(gametree, g2);
  game_add(gametree, g3);

  game_info(gametree);
  game_drop(gametree);
  return 0;
}

*/