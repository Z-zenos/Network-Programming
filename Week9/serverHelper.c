#include <stdlib.h>
#include <string.h>

#include "linkedlist.h"
#include "serverHelper.h"

ClientInfo *findClient(XOR_LL *client_list, int sock) {
  XOR_LL_ITERATOR itr = XOR_LL_ITERATOR_INITIALISER;
  XOR_LL_LOOP_HTT_RST(client_list, &itr) {
    ClientInfo *clnt = (ClientInfo *)itr.node_data.ptr;
    if(clnt->sock == sock) return clnt;
  }
  return NULL;
}

void addClient(XOR_LL *client_list, int sock, char *address) {
  ClientInfo *new_clnt = (ClientInfo *)malloc(sizeof(*new_clnt));
  new_clnt->sock = sock;
  strcpy(new_clnt->address, address);
  memset(new_clnt->username, 0, USN_L);
  xor_ll_push_tail(client_list, new_clnt, sizeof *new_clnt);
}

void removeClient(XOR_LL *client_list, int sock) {
  XOR_LL_ITERATOR itr = XOR_LL_ITERATOR_INITIALISER;
  XOR_LL_LOOP_HTT_RST(client_list, &itr) {
    ClientInfo *clnt = (ClientInfo *)itr.node_data.ptr;
    if(clnt->sock == sock)
      xor_ll_remove_node_iter(client_list, &itr);
  }
}