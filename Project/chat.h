
#ifndef _CHAT_H_
#define _CHAT_H_

typedef struct Chat {
  int id;
  int player1_id;
  int player2_id;

} Chat;

int send_msg(int );