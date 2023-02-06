#include <stdio.h>
#include <string.h>

#include "config.h"
#include "game.h"
#include "http.h"
#include "player.h"
#include "utils.h"

int chat(MYSQL *conn, ClientAddr clnt_addr, GameTree *gametree, PlayerTree *playertree, Message *msg, int *receiver) {
  int player_id = atoi(map_val(msg->params, "player_id"));
  int game_id = atoi(map_val(msg->params, "game_id"));
  char dataStr[DATA_L], content[CHAT_L];
  memset(dataStr, '\0', DATA_L);
  memset(content, '\0', CHAT_L);

  // TODO: Get message content and validate
  sscanf(msg->content, "%[^\n]", content);
  if(
    msg->content_l == 0 || strlen(msg->content) == 0 ||
    msg->content_l > CHAT_L || strlen(msg->content) > CHAT_L
  ) {
    responsify(msg, "chat_fail", NULL);
    return FAILURE;
  }

  // TODO: Chat global
  if(game_id == 0) {
    sprintf(dataStr, "username=%s,content=%s", player_username(playertree, player_id), str_trim(content));
    receiver[0] = -1; // -1 mean sent for all player
    responsify(msg, "chat_global", dataStr);
    return SUCCESS;
  }

  // TODO: Find game room for player
  Game *game_found = game_find(gametree, game_id);

  if(!game_found) {
    responsify(msg, "game_null", NULL);
    return FAILURE;
  }

  receiver[0] = player_fd(playertree, game_found->player1_id == player_id ? game_found->player2_id : game_found->player1_id);
  sprintf(dataStr, "username=%s,content=%s", player_username(playertree, player_id), str_trim(content));
  responsify(msg, "chat_local", dataStr);
  return SUCCESS;
}