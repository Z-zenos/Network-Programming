#include "linkedlist.h"
#include "config.h"

typedef struct ClientInfo {
  int sock;
  char username[USN_L];
  char address[ADDR_L];
} ClientInfo;

ClientInfo *findClient(XOR_LL *, int);
void addClient(XOR_LL *, int, char *);
void removeClient(XOR_LL *, int);
