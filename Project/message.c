#include <stdio.h>

#include "message.h"
#include "env.h"
#include "log.h"

struct T3Message {
  Message messageCode;
  char *message;
};

struct T3Message messages[NUMBER_MESSAGES] = {
  { T3_INPUT_EMPTY, "Input empty" },
  { T3_INPUT_INVALID, "Input invalid" },
  { T3_INPUT_TOO_LONG, "Input too long" },
  { T3_EMAIL_INVALID, "Email invalid" },
  { T3_DATABASE_CONNECT_FAILED, "Connect to database failed" },
};

void t3_message(Message msgCode) {
  for(int i = 0; i < NUMBER_MESSAGES; i++) {
    if(messages[i].messageCode == msgCode) {
      log_warn(messages[i].message);
    }
  }
}

