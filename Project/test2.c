#include "http.h"

#include <stdio.h>
#include <string.h>
int main() {
  Message msg;
  char req[100] = "AUTH /account\r\nContent-Length: 4\r\nParams: id=1\r\n\r\n";
  m_parse(&msg, req);
  m_print(msg);
  printf("strlen: %lu", strlen(msg.body.content));
  return 0;
}