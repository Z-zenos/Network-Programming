#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

int main() {
  int sender_socket;
  struct sockaddr_in sender_address;

  sender_socket = socket(AF_INET, SOCK_DGRAM, 0);

  sender_address.sin_family = AF_INET;
  sender_address.sin_port = htons(9000);
  sender_address.sin_addr.s_addr = INADDR_ANY;

  bind(sender_socket, (struct sockaddr*) &sender_address, sizeof(sender_address));
  struct sockaddr_in broadcast_address;
  char broadcast_message[1024];

  broadcast_address.sin_family = AF_INET;
  broadcast_address.sin_port = htons(9000);
  broadcast_address.sin_addr.s_addr = inet_addr("255.255.255.255");

  strcpy(broadcast_message, "Hello from the sender");

  int broadcast_permission = 1;
  setsockopt(sender_socket, SOL_SOCKET, SO_BROADCAST, (void *) &broadcast_permission, sizeof(broadcast_permission));

  sendto(sender_socket, broadcast_message, strlen(broadcast_message), 0, (struct sockaddr*) &broadcast_address, sizeof(broadcast_address));

}