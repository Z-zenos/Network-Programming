#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

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
  logger(L_SUCCESS, 1, "Build game tree successfully...");
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
  for ( int i = 0; i < BOARD_S; ++i ){
    memcpy(game->board[i], new_game.board[i], sizeof new_game.board[i]);
  }

  ret = rbinsert(gametree, (void *)game);
  if (ret == 0) {
    logger(L_ERROR, 1, "Can't insert new game for players");
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
    logger(L_ERROR, 1, "Can't delete game id: ", itoa(id, 10));
    free(game);
    return -1;
  }

  return 0;
}

Game *game_find(GameTree *gametree, int id) {
  Game *game, game_find;

  game_find.id = id;
  game = rbfind(gametree, &game_find);
  if (!game) {
    return 0;
  }
  return game;
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
void game_handler(GameTree *gametree, PlayerTree *playertree, Request *req, Response *res) {
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
  char dataStr[100];


  // TODO: Check state game
  if(checkWinning(game->board, turn, game->col, game->row)) {
    sprintf(dataStr, "win=%d", player_id);
    responsify(res, 200, dataStr, "Player id win");

    Player *winner = player_find(playertree, player_id);
    opponent_id = game->player1_id == player_id ? game->player2_id : player_id;
    Player *losser = player_find(playertree, opponent_id);

    winner->achievement.win++;
    winner->achievement.points += 3;

    losser->achievement.loss++;
    losser->achievement.points -= 1;
    return;
  }

  sprintf(dataStr, "turn=%c&col=%d&row=%d", turn, col, row);
  responsify(res, 200, dataStr, NULL);
  return;
}

void game_create(MYSQL *conn, GameTree *gametree, Request *req, Response *res) {
  int player_id;

  // TODO: Get player id
  sscanf(req->header.params, "player_id=%d", &player_id);

  // TODO: Get last id
  int last_game_id = mysql_insert_id(conn);

  // TODO: random first turn for game board
  int r = rand() % 2;

  // TODO: Create game
  Game new_game = {
    .id = last_game_id + 1,
    .views = 0,
    .num_move = 0,
    .result = 0,
    .turn = (r == 1) ? 'X' : 'O',
    .player1_id = player_id
  };

  game_add(gametree, new_game);

  char dataStr[100];
  sprintf(dataStr, "game_id=%d&turn=%c", new_game.id, new_game.turn);
  responsify(res, 200, dataStr, "Create new game successfully");
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