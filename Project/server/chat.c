#include <stdio.h>
#include <string.h>

#include "game.h"
#include "config.h"
#include "player.h"

void chat(GameTree *gametree, PlayerTree *playertree, Request *req, Response *res) {
  int player_id, game_id;
  char msgStr[MESSAGE_L], dataStr[DATA_L], content[CONTENT_L];
  memset(msgStr, '\0', MESSAGE_L);
  memset(dataStr, '\0', DATA_L);
  memset(content, '\0', CONTENT_L);

  // TODO: Get player id and game id from user
  if(sscanf(req->header.params, "game_id=%d&player_id=%d", &game_id, &player_id) != 2) {
    responsify(res, 400, NULL, NULL, "Bad request. Usage: CHAT /chat game_id=...&player_id=...", SEND_ME);
    return;
  }

  // TODO: Get message content and validate
  sscanf(req->body.content, "%[^\n]", content);
  if(req->header.content_l == 0 || strlen(req->body.content) == 0) {
    responsify(res, 400, "chat_fail", NULL, "Message empty", SEND_ME);
    return;
  }

  if(req->header.content_l > CONTENT_L || strlen(req->body.content) > CONTENT_L) {
    responsify(res, 400, "chat_fail", NULL, "Message too long", SEND_ME);
    return;
  }

  // TODO: Chat global
  if(game_id == 0) {
    sprintf(dataStr, "username=%s&content=%s", player_username(playertree, player_id), content);
    responsify(res, 200, "chat_global", dataStr, "Chat successfully", SEND_ALL);
    return;
  }

  // TODO: Find game room for player
  Game *game_found = game_find(gametree, game_id);

  if(!game_found) {
    sprintf(msgStr, "Game [%d] does not exist", game_id);
    responsify(res, 400, NULL, NULL, msgStr, SEND_ME);
    return;
  }

  // TODO: Check if the sender of the message is 1 of 2 players
  if(game_found->player1_id != player_id && game_found->player2_id != player_id) {
    responsify(res, 403, NULL, NULL, "Spectator can't chat in game", SEND_ME);
    return;
  }

  sprintf(dataStr, "username=%s&content=%s", player_username(playertree, player_id), content);
  responsify(res, 200, "chat_local", dataStr, "Chat successfully", SEND_JOINER);
}