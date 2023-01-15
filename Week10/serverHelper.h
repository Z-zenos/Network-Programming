#include "linkedlist.h"
#include "constants.h"

typedef struct ClientInfo {
  int sock;
  char username[MAX_USERNAME];
  char address[MAX_HOMEPAGE];
} ClientInfo;

ClientInfo *findClient(XOR_LL *, int);
void addClient(XOR_LL *, int, char *);
void removeClient(XOR_LL *, int);

