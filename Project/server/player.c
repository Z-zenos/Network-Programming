#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mysql/mysql.h>
#include <ctype.h>

#include "player.h"
#include "http.h"
#include "config.h"
#include "rbtree.h"
#include "utils.h"


static int player_cmp(const void *p1, const void *p2) {
  Player *player1, *player2;

  player1 = (Player *)p1;
  player2 = (Player *)p2;

  if(player1->id > player2->id) return 1;
  else if(player1->id < player2->id) return -1;
  else return 0;
}

static void *player_dup(void *p) {
  void *dup_p;

  dup_p = calloc(1, sizeof(Player));
  memmove(dup_p, p, sizeof(Player));

  return dup_p;
}

static void player_rel(void *p) { free(p); }

int player_add(PlayerTree *playertree, Player new_player) {
  int ret;

  Player *player;
  player = calloc(1, sizeof(Player));

  player->id = new_player.id;
  strcpy(player->username, new_player.username);
  strcpy(player->password, new_player.password);
  strcpy(player->avatar, new_player.avatar);
  player->game = new_player.game;
  player->sock = 0;
  player->achievement = new_player.achievement;
  memcpy(player->friends, new_player.friends, FRIEND_L);

  ret = rbinsert(playertree, (void *)player);
  if (ret == 0) {
    logger(L_ERROR, "Can't insert new player for players");
    free(player);
    return -1;
  }

  return 0;
}

PlayerTree *player_build(MYSQL *conn) {
  rbtree_t *rbtree;
  rbtree = rbnew(player_cmp, player_dup, player_rel);

  // TODO: Read players data from database
  char query[QUERY_L] = "SELECT players.*, GROUP_CONCAT(friends.friend_id SEPARATOR ',') FROM players INNER JOIN friends ON players.id = friends.player_id GROUP BY players.id";

  if (mysql_query(conn, query)) {
    logger(L_ERROR, mysql_error(conn));
    return NULL;
  }

  MYSQL_RES *qres = mysql_store_result(conn);
  if(!qres->row_count) {
    mysql_free_result(qres);
    return NULL;
  }

  MYSQL_ROW row;
  Player player;
  int j = 0;

  while ((row = mysql_fetch_row(qres))) {
    j = 0;
    player.id = atoi(row[0]);
    strcpy(player.username, row[1]);
    strcpy(player.password, row[2]);
    strcpy(player.avatar, row[3]);
    player.game = atoi(row[4]);
    player.achievement.win = atoi(row[5]);
    player.achievement.loss = atoi(row[6]);
    player.achievement.draw = atoi(row[7]);
    player.achievement.streak = atoi(row[8]);
    player.achievement.points = atoi(row[9]);
    player.sock = 0;

    // TODO: Read friend
    char **friend_ids = str_split(row[10], ',');
    while (friend_ids[j]) {
      player.friends[j] = atoi(friend_ids[j]);
      j++;
    }

    player_add(rbtree, player);
  }

  mysql_free_result(qres);
  logger(L_SUCCESS, "Build playertree successfully...");
  return rbtree;
}

Player *player_find(PlayerTree *playertree, int player_id) {
  Player *player, player_find;

  player_find.id = player_id;
  player = rbfind(playertree, &player_find);
  if (!player) {
    return NULL;
  }
  return player;
}

void player_print(Player *player) {
  printf(
    "id %d - username: %s - password: %s - avatar: %s - game: %d - win: %d - draw: %d - loss: %d - streak: %d - points: %d\n",
    player->id, player->username, player->password, player->avatar, player->game,
    player->achievement.win, player->achievement.draw, player->achievement.loss, player->achievement.streak, player->achievement.points
    );
}

void player_info(PlayerTree *playertree) {
  Player *player;

  rbtrav_t *rbtrav;
  rbtrav = rbtnew();
  player = rbtfirst(rbtrav, playertree);
  player_print(player);

  while ((player = rbtnext(rbtrav)) != NULL) {
    player_print(player);
  }
}

int player_fd(PlayerTree *playertree, int player_id) {
  Player *player_found = player_find(playertree, player_id);
  return player_found->sock;
}

char *player_username(PlayerTree *playertree, int player_id) {
  Player *player_found = player_find(playertree, player_id);
  return player_found->username;
}

int my_rank(MYSQL *conn, int player_id, char *dataStr) {
  // TODO: QUERY find rank of specified player id
  char query[QUERY_L] = ""
    "select er.id, er.username, er.avatar, er.game, er.win, er.draw, er.loss, er.points, (@rank := if(@points = points, @rank, if(@points := points, @rank + 1, @rank + 1))) as ranking "
    "from players er cross join (select @rank := 0, @points := -1) params "
    "order by points desc";

  if (mysql_query(conn, query)) {
    logger(L_ERROR, "Query to database failed");
    logger(L_ERROR, mysql_error(conn));
    return -1;
  }

  MYSQL_RES *qres = mysql_store_result(conn);
  if(!qres->row_count) {
    mysql_free_result(qres);
    return -1;
  }

  // TODO: Count number of records in db
  MYSQL_ROW row;
  char *idStr = itoa(player_id, 10);
  char line[DATA_L];
  memset(line, '\0', DATA_L);

  while ((row = mysql_fetch_row(qres))) {
    if(strcmp(row[0], idStr) == 0) {
      sprintf(line, "username=%s&avatar=%s&game=%d&win=%d&draw=%d&loss=%d&points=%d", row[1], row[2], atoi(row[3]), atoi(row[4]), atoi(row[5]), atoi(row[6]), atoi(row[7]));
      strcat(dataStr, line);
      mysql_free_result(qres);
      return atoi(row[8]);
    }
  }

  mysql_free_result(qres);
  return -1;
}

void rank(MYSQL *conn, Request *req, Response *res) {
  int player_id;

  // TODO: Get player id from request
  if(sscanf(req->header.params, "player_id=%d", &player_id) <= 0) {
    responsify(res, 400, NULL, NULL, "Bad request. Usage: GET /rank player_id=...", SEND_ME);
    return;
  }

  // TODO: QUERY follow points from database
  char query[QUERY_L] = "SELECT id, username, avatar, win, loss, points FROM players ORDER BY points DESC LIMIT 10";

  if (mysql_query(conn, query)) {
    logger(L_ERROR, "Query to database failed");
    logger(L_ERROR, mysql_error(conn));
    return;
  }

  MYSQL_RES *qres = mysql_store_result(conn);
  if(!qres->row_count) {
    mysql_free_result(qres);
    return;
  }

  // TODO: Count number of records in db
  int total_players = mysql_num_rows(qres), i = 0;
  MYSQL_ROW row;
  Player player[total_players];
  char dataStr[DATA_L];
  memset(dataStr, '\0', sizeof dataStr);

  while ((row = mysql_fetch_row(qres))) {
    player[i].id = atoi(row[0]);
    strcpy(player[i].username, row[1]);
    strcpy(player[i].avatar, row[2]);
    player[i].achievement.win = atoi(row[3]);
    player[i].achievement.loss = atoi(row[4]);
    player[i].achievement.points = atoi(row[5]);
    char line[1000];
    sprintf(
      line,
      "id=%d&username=%s&avatar=%s&win=%d&loss=%d&points=%d;",
      player[i].id, player[i].username, player[i].avatar, player[i].achievement.win,
      player[i].achievement.loss, player[i].achievement.points
    );
    strcat(dataStr, line);
    i++;
  }

  mysql_free_result(qres);
  my_rank(conn, player_id, dataStr);
  responsify(res, 200, "rank", dataStr, "Get rank successfully", SEND_ME);
}

void profile(MYSQL *conn, Request *req, Response *res) {
  char key[USERNAME_L];
  char msgStr[MESSAGE_L], dataStr[DATA_L], tmp[DATA_L];
  memset(msgStr, '\0', MESSAGE_L);
  memset(dataStr, '\0', DATA_L);

  // TODO: Get key from request
  if(sscanf(req->header.params, "key=%s", key) <= 0) {
    responsify(res, 400, NULL, NULL, "Bad request. Usage: GET /profile key=[player_id | username]", SEND_ME);
    return;
  }

  // TODO:
  int i, key_l = strlen(key), type = 0;
  for(i = 0; i < key_l; i++) {
    if(isalpha(key[i])) {
      type = 1;
      break;
    }
  }

  // TODO: QUERY
  char query[QUERY_L];
  memset(query, '\0', QUERY_L);

  switch (type) {
    // if key = 0 -> info is player id
    case 0:
      sprintf(query, "SELECT id, username, avatar, game, win, draw, loss, points FROM players WHERE id = %d", atoi(key));
      break;

    // if key = 1 -> info is username
    case 1:
      sprintf(query, "SELECT id, username, avatar, game, win, draw, loss, points FROM players WHERE username = '%s'", key);
      break;

    default: break;
  }

  if (mysql_query(conn, query)) {
    logger(L_ERROR, "Query to database failed");
    logger(L_ERROR, mysql_error(conn));
    return;
  }

  MYSQL_RES *qres = mysql_store_result(conn);
  if(!qres->row_count) {
    sprintf(msgStr, "Player [%s] does not exist", key);
    responsify(res, 400, NULL, NULL, msgStr, SEND_ME);
    mysql_free_result(qres);
    return;
  }

  MYSQL_ROW row;
  while ((row = mysql_fetch_row(qres))) {
    sprintf(
      dataStr,
      "username=%s&avatar=%s&game=%d&win=%d&draw=%d&loss=%d&streak=%d&points=%d&rank=%d",
      row[1], row[2], atoi(row[3]), atoi(row[4]), atoi(row[5]), atoi(row[6]), atoi(row[7]), atoi(row[8]), my_rank(conn, type ? atoi(row[0]) : atoi(key), tmp)
    );
  }

  mysql_free_result(qres);
  sprintf(msgStr, "Get info of player [%s] successfully", key);
  responsify(res, 200, NULL, dataStr, msgStr, SEND_ME);
}

void friend_check(MYSQL *conn, Request *req, Response *res) {
  int player_id, friend_id;

  // TODO: Get player id and friend id from request
  if(sscanf(req->header.params, "player_id=%d&friend_id=%d", &player_id, &friend_id) != 2) {
    responsify(res, 400, NULL, NULL, "Bad request. Usage: FRIEND /friend/check player_id=...&friend_id=...", SEND_ME);
    return;
  }

  // TODO: QUERY check friend in database
  char query[QUERY_L];
  memset(query, '\0', QUERY_L);
  sprintf(query, "SELECT * FROM friends WHERE player_id = %d AND friend_id = %d", player_id, friend_id);

  if (mysql_query(conn, query)) {
    responsify(res, 400, "friend_check", "is_friend=0", "The two are not friends yet", SEND_ME);
    return;
  }

  MYSQL_RES *qres = mysql_store_result(conn);
  if(!qres->row_count) {
    responsify(res, 400, "friend_check", "is_friend=0", "The two are not friends yet", SEND_ME);
    mysql_free_result(qres);
    return;
  }

  mysql_free_result(qres);
  responsify(res, 200, "friend_check", "is_friend=1", "The two were already friends", SEND_ME);
}

void friend_list(MYSQL *conn, Request *req, Response *res) {
  int player_id;

  // TODO: Get player id and friend id from request
  if(sscanf(req->header.params, "player_id=%d", &player_id) != 1) {
    responsify(res, 400, NULL, NULL, "Bad request. Usage: FRIEND /friend/list player_id=...", SEND_ME);
    return;
  }

  // TODO: QUERY check friend in database
  char query[QUERY_L];
  memset(query, '\0', QUERY_L);
  sprintf(
    query,
    "SELECT players.id, players.username, players.avatar FROM players, friends WHERE players.id = friends.friend_id AND friends.player_id = %d",
    player_id
  );

  if (mysql_query(conn, query)) {
    logger(L_ERROR, "Query to database failed");
    return;
  }

  MYSQL_RES *qres = mysql_store_result(conn);
  if(!qres->row_count) {
    responsify(res, 400, "friend_list", "", "Friend list is empty", SEND_ME);
    mysql_free_result(qres);
    return;
  }

  MYSQL_ROW row;
  char line[100], dataStr[DATA_L];
  memset(line, '\0', 100);
  memset(dataStr, '\0', DATA_L);

  while ((row = mysql_fetch_row(qres))) {
    sprintf(line, "id=%d&username=%s&avatar=%s;", atoi(row[0]), row[1], row[2]);
    strcat(dataStr, line);
  }
  dataStr[strlen(dataStr) - 1] = '\0';

  mysql_free_result(qres);
  responsify(res, 200, "friend_list", dataStr, "Get friend list successfully", SEND_ME);
}

void friend_request(PlayerTree *playertree, Request *req, Response *res) {
  int player_id, friend_id;

  // TODO: Get player id from request
  if(sscanf(req->header.params, "player_id=%d&friend_id=%d", &player_id, &friend_id) != 2) {
    responsify(res, 400, NULL, NULL, "Bad request. Usage: FRIEND /friend/add player_id=...&friend_id=...", SEND_ME);
    return;
  }

  Player *friend = player_find(playertree, friend_id);
  if(!friend) {
    responsify(res, 400, NULL, NULL, "Friend not found", SEND_ME);
    return;
  }
//  int friend_fd = friend->sock;

  responsify(res, 200, "friend_add", "is_friend=1", "Add new friend successfully", SEND_ME);
}

void friend_accept(MYSQL *conn, PlayerTree *playertree, Request *req, Response *res) {
  int player_id, friend_id, i, accept;
  // TODO: Get player id from request
  if(sscanf(req->header.params, "player_id=%d&friend_id=%d&accept=%d", &player_id, &friend_id, &accept) != 3) {
    responsify(res, 400, NULL, NULL, "Bad request. Usage: FRIEND /friend/accept player_id=...&friend_id=...", SEND_ME);
    return;
  }

  // TODO: QUERY follow points from database
  char query[QUERY_L];
  memset(query, '\0', QUERY_L);
  sprintf(query, "INSERT INTO friends (player_id, friend_id) VALUES (%d, %d)", player_id, friend_id);

  if (mysql_query(conn, query)) {
    responsify(res, 400, NULL, NULL, "Add friend failed", SEND_ME);
    return;
  }

  char dataStr[DATA_L];
  memset(dataStr, '\0', DATA_L);

  if(!accept) {
    responsify(res, 400, "friend_accept", "accept=0", "Deny friend", SEND_ME);
    return;
  }

  Player *me = player_find(playertree, player_id);
  Player *friend = player_find(playertree, friend_id);
  for(i = 0; i < FRIEND_L; i++)
    if(me->friends[i] == 0) me->friends[i] = friend_id;
  for(i = 0; i < FRIEND_L; i++)
    if(friend->friends[i] == 0) friend->friends[i] = player_id;

  memset(dataStr, '\0', DATA_L);
  sprintf(dataStr, "friend_id=%d&username=%s", friend_id, player_username(playertree, friend_id));
  responsify(res, 200, "friend_accept", dataStr, "Add new friend successfully", SEND_ME);
}