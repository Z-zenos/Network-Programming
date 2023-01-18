
#ifndef __CHAT_H__
#define __CHAT_H_

typedef struct Chat {
  int id;
  int player1_id;
  int player2_id;

} Chat;

int send_msg(int );