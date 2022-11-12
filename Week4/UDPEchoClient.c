#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <netdb.h>

#define MAXSTRINGLENGTH 255

void DieWithUserMessage(const char *msg, const char *detail) {
  fputs(msg, stderr);
  fputs(": ", stderr);
  fputs(detail, stderr);
  fputc('\n', stderr);
  exit(1);
}

void DieWithSystemMessage(const char *msg) {
  perror(msg);
  exit(1);
}

bool SockAddrsEqual(const struct sockaddr *addr1, const struct sockaddr *addr2) {
  if (addr1 == NULL || addr2 == NULL)
    return addr1 == addr2;
  else if (addr1->sa_family != addr2->sa_family)
    return false;
  else if (addr1->sa_family == AF_INET) {
    struct sockaddr_in *ipv4Addr1 = (struct sockaddr_in *) addr1;
    struct sockaddr_in *ipv4Addr2 = (struct sockaddr_in *) addr2;
    return ipv4Addr1->sin_addr.s_addr == ipv4Addr2->sin_addr.s_addr
           && ipv4Addr1->sin_port == ipv4Addr2->sin_port;
  } else if (addr1->sa_family == AF_INET6) {
    struct sockaddr_in6 *ipv6Addr1 = (struct sockaddr_in6 *) addr1;
    struct sockaddr_in6 *ipv6Addr2 = (struct sockaddr_in6 *) addr2;
    return memcmp(&ipv6Addr1->sin6_addr, &ipv6Addr2->sin6_addr,
                  sizeof(struct in6_addr)) == 0 && ipv6Addr1->sin6_port
                                                   == ipv6Addr2->sin6_port;
  } else
    return false;
}

int main(int argc, char *argv[]) {
  // Check correct number of arguments
  if(argc < 3 || argc > 4)
    DieWithUserMessage("Parameter(s)", "<Server Address/Name> <Echo Word> [<Server Port/Service>]");

  char *server = argv[1]; // First arg: Server address/name

  // Third arg (option): server port / service
  char *servPort = argc == 4 ? argv[3] : "echo";

  // Tell the system what kind(s) of address info we want - Config address
  struct addrinfo addrConfig; // Criteria for address match
  memset(&addrConfig, 0, sizeof(addrConfig)); // Zero out structure
  addrConfig.ai_family = AF_UNSPEC; // Any address family

  // For the following fields, a zero value means "don't care"
  addrConfig.ai_socktype = SOCK_DGRAM; // Only datagram sockets
  addrConfig.ai_protocol = IPPROTO_UDP; // Only UDP protocol

  struct addrinfo *servAddr; // List of server address
  // Get server information via servAddr
  int rtnVal = getaddrinfo(server, servPort, &addrConfig, &servAddr);
  if(rtnVal != 0)
    DieWithUserMessage("getaddrinfo() failed", gai_strerror(rtnVal));

  // Create a datagram / UDP socket
  int sock = socket(servAddr->ai_family, servAddr->ai_socktype, servAddr->ai_protocol); // Socket descriptor for client
  if(sock < 0)
    DieWithSystemMessage("socket() failed");

//  char *echoString = argv[2]; // Second arg: word to echo
  char echoString[MAXSTRINGLENGTH + 1];
  while(1) {

    printf("Message: ");
    scanf("%s", echoString);
    size_t echoStringLen = strlen(echoString); // Length of echo string

    // Check input length
    if (echoStringLen > MAXSTRINGLENGTH) {
      DieWithUserMessage(echoString, "string too long");
    }
    // Send the string to the server
    /*
      The first call to sendto() also assigns an arbitrarily chosen local port number,
      not in use by any other socket,
      to the socket identified by sock,
      because we have not previously bound the socket to a port number.
      We do not care what the chosen port number is,
      but the server will use it to send the echoed message back to us.
     * */
    // the number of bytes sent.
    ssize_t numBytes = sendto(sock, echoString, echoStringLen, 0, servAddr->ai_addr, servAddr->ai_addrlen);
    if (numBytes < 0)
      DieWithSystemMessage("sendto() failed");
    else if (numBytes != echoStringLen)
      DieWithUserMessage("sendto() error: ", " send unexpected number of bytes");

    // Receive a response
    // Source address of server - use sockaddr_storage because we don't know what type address of server is IPv4 or IPv6.
    struct sockaddr_storage fromAddr;

    // Set length of from address structure (in-out parameter)
    socklen_t fromAddrLen = sizeof(fromAddr);
    char buffer[MAXSTRINGLENGTH + 1]; // I/O Buffer
    numBytes = recvfrom(sock, buffer, MAXSTRINGLENGTH, 0, (struct sockaddr *) &fromAddr, &fromAddrLen);
    if (numBytes < 0)
      DieWithSystemMessage("recvfrom() failed");
    else if (numBytes != echoStringLen)
      DieWithUserMessage("recvfrom() error: ", "received unexpected number of bytes");

    // Verify reception from expected source
    if (!SockAddrsEqual(servAddr->ai_addr, (struct sockaddr *) &fromAddr))
      DieWithUserMessage("recvfrom()", "received a packet from unknown source");
    buffer[echoStringLen] = '\0';
    printf("Received: %s\n", buffer);
  }
  freeaddrinfo(servAddr);

  close(sock);
  exit(0);
}