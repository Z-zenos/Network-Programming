#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "config.h"
#include "player.h"

void chat(GameTree *gametree, PlayerTree *playertree, Request *req, Response *res) {
  int player_id, game_id, client_fd;
  char msgStr[MESSAGE_L], dataStr[DATA_L], content[CONTENT_L];
  memset(msgStr, '\0', MESSAGE_L);
  memset(dataStr, '\0', DATA_L);
  memset(content, '\0', CONTENT_L);

  // TODO: Get player id and game id from user
  if(sscanf(req->header.params, "sock=%d&game_id=%d&player_id=%d", &client_fd, &game_id, &player_id) != 2) {
    responsify(res, 400, NULL, "Bad request. Usage: CHAT /chat sock=...&game_id=...&player_id=...");
    return;
  }

  // TODO: Find game room for player
  Game *game_found = game_find(gametree, game_id);

  if(!game_found) {
    sprintf(msgStr, "Game [%d] does not exist", game_id);
    responsify(res, 400, NULL, msgStr);
    return;
  }

  // TODO: Check if the sender of the message is 1 of 2 players
  if(game_found->player1_id != player_id && game_found->player2_id != player_id) {
    responsify(res, 403, NULL, "Spectator can't chat in game");
    return;
  }

  // TODO: Get message content and validate
  sscanf(req->body.content, "%[^\n]", content);
  if(req->header.content_l == 0 || strlen(req->body.content) == 0) {
    responsify(res, 400, NULL, "Message empty");
    return;
  }

  if(req->header.content_l > CONTENT_L || strlen(req->body.content) > CONTENT_L) {
    responsify(res, 400, NULL, "Message too long");
    return;
  }

  sprintf(dataStr, "username=%s&message=%s", player_username(playertree, player_id), content);
  responsify(res, 200, dataStr, "Chat successfully");

  // TODO: Send to all spectators and remain player
  int remain_player_id = (game_found->player1_id == player_id ? player_id : game_found->player2_id);
  // If there is a 2nd player -> send
  if(remain_player_id) send_res(player_fd(playertree, remain_player_id), *res);

  int curr_fd;
  for(int i = 0; i < MAX_SPECTATOR; i++) {
    if(game_found->spectators[i]) {
      curr_fd = player_fd(playertree, game_found->spectators[i]);
      send_res(curr_fd, *res);
    }
  }
}