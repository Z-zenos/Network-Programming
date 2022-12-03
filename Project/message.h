#ifndef __MESSAGE_H__
#define __MESSAGE_H__

typedef enum {
  T3_INPUT_EMPTY,
  T3_INPUT_INVALID,
  T3_INPUT_TOO_LONG,
  T3_EMAIL_INVALID,

  T3_DATABASE_CONNECT_FAILED
} Message;

void t3_message(Message);

#endif