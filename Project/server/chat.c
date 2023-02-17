#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "clist.h"
#include "config.h"
#include "game.h"
#include "http.h"
#include "player.h"
#include "utils.h"
#include "chat.h"

static int bad_words_cmp(const void *p1, const void *p2) {
  Word *bad_word1, *bad_word2;

  bad_word1 = (Word *)p1;
  bad_word2 = (Word *)p2;

  if(strcmp(bad_word1->word, bad_word2->word) > 0) return 1;
  if(strcmp(bad_word1->word, bad_word2->word) < 0) return -1;
  else return 0;
}

static void *bad_words_dup(void *p) {
  void *dup_p;

  dup_p = calloc(1, sizeof(Word));
  memmove(dup_p, p, sizeof(Word));

  return dup_p;
}

static void bad_words_rel(void *p) { free(p); }

void bad_words_drop(BadWordsStorage *bad_words_storage) { rbdelete(bad_words_storage); }

int bad_words_add(BadWordsStorage *bad_words_storage, char *new_bad_word) {
  int ret;

  Word *bad_word = calloc(1, sizeof(Word));
  strcpy(bad_word->word, new_bad_word);

  ret = rbinsert(bad_words_storage, (void *)bad_word);
  if (ret == 0) {
    free(bad_word);
    return -1;
  }

  return 0;
}

BadWordsStorage *bad_words_build() {
  rbtree_t *rbtree;
  rbtree = rbnew(bad_words_cmp, bad_words_dup, bad_words_rel);

  // TODO: Read bad word data from vietnamese_bad_words.txt
  char word[VAL_L];
  memset(word, '\0', VAL_L);
  FILE *bwf = fopen("vietnamese_bad_words.txt", "r");

  if(!bwf) {
    logger(L_ERROR, "Can't read file vietnamese bad words");
    return NULL;
  }

  Word bad_word;
  rewind(bwf);
  while (!feof(bwf)) {
    fgets(word, sizeof(word), bwf);
    word[strlen(word) - 1] = '\0';
    strcpy(bad_word.word, word);
    bad_words_add(rbtree, bad_word.word);
  }

  fclose(bwf);
  logger(L_SUCCESS, "Build bad words storage successfully...");
  return rbtree;
}

char *bad_words_find(BadWordsStorage *bad_words_storage, char *word) {
  Word *bad_word, bad_word_find;

  strcpy(bad_word_find.word, word);
  bad_word = rbfind(bad_words_storage, &bad_word_find);

  return !bad_word ? NULL : bad_word->word;
}

char *bad_words_filter(BadWordsStorage *bad_words_storage, char *content) {
  if(!content) return NULL;
  char *content_tmp = (char *)calloc(CONTENT_L, sizeof(char));

  char **words = str_split(content, ' ');
  int i = 0;
  while (words[i]) {
    strcat(content_tmp, bad_words_find(bad_words_storage, words[i]) ? "***" : words[i]);
    strcat(content_tmp, " ");
    i++;
  }
  content_tmp[strlen(content_tmp) - 1] = '\0';
  return content_tmp;
}

int chat(
  GameTree *gametree, PlayerTree *playertree, CList *queue_msg,
  BadWordsStorage *bad_words_storage, Message *msg, int *receiver
) {
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
    sprintf(dataStr, "username=%s,content=%s", player_username(playertree, player_id), "Tin nhắn không hợp lệ (dài quá quá 2000 ký tự)");
    responsify(msg, "chat_fail", dataStr);
    return FAILURE;
  }


  // TODO: Chat global
  if(game_id == 0) {
    // TODO: Filter bad words and trim space
    sprintf(dataStr, "username=%s,content=%s", player_username(playertree, player_id), str_trim(bad_words_filter(bad_words_storage, content)));

    // TODO: Add global message to queue for send to players who come in later
    Chat ct;
    strcpy(ct.content, dataStr);
    int count = queue_msg->count(queue_msg);
    queue_msg->insert(queue_msg, (void *)&ct, count);

    receiver[0] = -1; // -1 mean sent for all player
    responsify(msg, "chat_global", dataStr);
    return SUCCESS;
  }

  // TODO: Chat local
  // TODO: Find game room for player
  Game *game_found = game_find(gametree, game_id);
  if(!game_found) {
    responsify(msg, "game_null", NULL);
    return FAILURE;
  }

  receiver[0] = player_fd(playertree, game_found->player1_id == player_id ? game_found->player2_id : game_found->player1_id);
  sprintf(dataStr, "username=%s,content=%s", player_username(playertree, player_id), str_trim(bad_words_filter(bad_words_storage, content)));
  responsify(msg, "chat_local", dataStr);
  return SUCCESS;
}