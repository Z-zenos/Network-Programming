#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "http.h"
#include "constants.h"
#include "error.h"

#define NUM_OF_CODE 10

struct HTTPStatus {
    HttpCode httpCode;
    int code;
    char *message;
};

struct HTTPStatus statuses[NUM_OF_CODE] = {
  { _200_, 200, "OK" },
  { _201_, 201, "Created" },
  { _202_, 202, "Accepted" },
  { _204_, 204, "No Content" },
  { _400_, 400, "Bad Request" },
  { _401_, 401, "Unauthorized" },
  { _403_, 403, "Forbidden" },
  { _404_, 404, "Not Found" },
  { _500_, 500, "Internal Server Error" },
};

int http_code(HttpCode code) {
  for (int i = 0; i < NUM_OF_CODE; i++)
    if (statuses[i].httpCode == code) {
      return statuses[i].code;
    }
  return 0;
}

char *http_message(HttpCode code) {
  for (int i = 0; i < NUM_OF_CODE; i++)
    if (statuses[i].httpCode == code) {
      return statuses[i].message;
    }
  return NULL;
}

int sock;
struct addrinfo *servAddr;
struct sockaddr_storage clntAddr; // Client address
ssize_t numBytesRcvd;

void http_clear(char *method, char *request, char *response) {
  memset(method, 0, 10);
  memset(request, 0, MAX_REQUEST_LENGTH);
  memset(response, 0, MAX_RESPONSE_LENGTH);
}

bool compare_sockaddr(const struct sockaddr *addr1, const struct sockaddr *addr2) {
  if (addr1 == NULL || addr2 == NULL)
    return addr1 == addr2;
  else if (addr1->sa_family != addr2->sa_family)
    return false;
  else if (addr1->sa_family == AF_INET) {
    struct sockaddr_in *ipv4Addr1 = (struct sockaddr_in *) addr1;
    struct sockaddr_in *ipv4Addr2 = (struct sockaddr_in *) addr2;
    return ipv4Addr1->sin_addr.s_addr == ipv4Addr2->sin_addr.s_addr && ipv4Addr1->sin_port == ipv4Addr2->sin_port;
  }
  else if (addr1->sa_family == AF_INET6) {
    struct sockaddr_in6 *ipv6Addr1 = (struct sockaddr_in6 *) addr1;
    struct sockaddr_in6 *ipv6Addr2 = (struct sockaddr_in6 *) addr2;
    return memcmp(&ipv6Addr1->sin6_addr, &ipv6Addr2->sin6_addr, sizeof(struct in6_addr)) == 0 && ipv6Addr1->sin6_port == ipv6Addr2->sin6_port;
  }
  else return false;
}

void print_socketaddr(const struct sockaddr *address, FILE *stream) {
  // Test for address and stream
  if (address == NULL || stream == NULL)
    return;

  void *numericAddress; // Pointer to binary address
  // Buffer to contain result (IPv6 sufficient to hold IPv4)
  char addrBuffer[INET6_ADDRSTRLEN];
  in_port_t port; // Port to print
  // Set pointer to address based on address family
  switch (address->sa_family) {
    case AF_INET:
      numericAddress = &((struct sockaddr_in *) address)->sin_addr;
      port = ntohs(((struct sockaddr_in *) address)->sin_port);
      break;
    case AF_INET6:
      numericAddress = &((struct sockaddr_in6 *) address)->sin6_addr;
      port = ntohs(((struct sockaddr_in6 *) address)->sin6_port);
      break;
    default:
      fputs("[unknown type]", stream); // Unhandled type
      return;
  }
  // Convert binary to printable address
  if (inet_ntop(address->sa_family, numericAddress, addrBuffer, sizeof(addrBuffer)) == NULL)
    fputs("[invalid address]", stream); // Unable to convert
  else {
    fprintf(stream, "%s", addrBuffer);
    if (port != 0) // Zero not valid in any socket addr
      fprintf(stream, "-%u", port);
  }
}

void requestify(char *method, char *request) {
  /* TEMPLATE: METHOD REQUEST */
  char request_tpl[MAX_REQUEST_LENGTH];
  sprintf(request_tpl, "%s %s", method, request);
  strcpy(request, request_tpl);
}

bool parse_request(char *method, char *request) {
  sscanf(request, "%s %[^\n]s", method, request);
  return SUCCESS;
}

int server_init_connect(char *service) {
  // Config the server address structure
  struct addrinfo addrConfig;
  memset(&addrConfig, 0, sizeof(addrConfig)); // Zero out structure
  addrConfig.ai_family = AF_UNSPEC; // Any address family
  addrConfig.ai_flags = AI_PASSIVE; // Accept on any address/port
  addrConfig.ai_socktype = SOCK_DGRAM; // Only datagram socket
  addrConfig.ai_protocol = IPPROTO_UDP; // Only UDP socket

  //  struct addrinfo *servAddr; // List of server addresses
  int rtnVal = getaddrinfo(NULL, service, &addrConfig, &servAddr);
  if (rtnVal != 0) {
    err_error(ERR_SERVER_NOT_FOUND);
    return FAIL;
  }

  // Create socket for incoming connections
  sock = socket(servAddr->ai_family, servAddr->ai_socktype, servAddr->ai_protocol);
  if (sock < 0) {
    err_error(ERR_INITIALIZE_SOCKET_FAILED);
    return FAIL;
  }

  // Bind to the local address
  if (bind(sock, servAddr->ai_addr, servAddr->ai_addrlen) < 0) {
    err_error(ERR_INITIALIZE_SOCKET_FAILED);
    return FAIL;
  }

  // Free address list allocated by getaddrinfo()
  freeaddrinfo(servAddr);
  return SUCCESS;
}

int client_init_connect(char *server, char *port) {

  // Tell the system what kind(s) of address info we want - Config address
  struct addrinfo addrConfig; // Criteria for address match
  memset(&addrConfig, 0, sizeof(addrConfig)); // Zero out structure
  addrConfig.ai_family = AF_UNSPEC; // Any address family

  // For the following fields, a zero value means "don't care"
  addrConfig.ai_socktype = SOCK_DGRAM; // Only datagram sockets
  addrConfig.ai_protocol = IPPROTO_UDP; // Only UDP protocol


  // Get server information via servAddr
  int rtnVal = getaddrinfo(server, port, &addrConfig, &servAddr);
  if (rtnVal != 0) {
    err_error(ERR_SERVER_NOT_FOUND);
    return FAIL;
  }

  // Create a datagram / UDP socket
  sock = socket(servAddr->ai_family, servAddr->ai_socktype, servAddr->ai_protocol); // Socket descriptor for client
  if (sock < 0) {
    err_error(ERR_INITIALIZE_SOCKET_FAILED);
    return FAIL;
  }
  return SUCCESS;
}

int get_request(char *method, char *request) {
  // Set Length of client address structure (in-out parameter)
  socklen_t clntAddrLen = sizeof(clntAddr);

  // Size of received message DEAL REQUEST FROM CLIENT
  numBytesRcvd = recvfrom(sock, request, MAX_MESSAGE, 0, (struct sockaddr *) &clntAddr, &clntAddrLen);
  if (numBytesRcvd < 0) {
    err_error(ERR_GET_REQUEST_FAILED);
    return FAIL;
  }

  request[numBytesRcvd] = '\0';

  // Split method and pure request
  if(!parse_request(method, request)) {
    err_error(ERR_SERVER_ERROR);
    return FAIL;
  }

  fputs("Handling client ", stdout);
  print_socketaddr((struct sockaddr *) &clntAddr, stdout);
  fputc('\n', stdout);
  printf("REQUEST: %s\n", request);
  return SUCCESS;
}

int get_response(char *request, char *response) {
  // Receive a response
  struct sockaddr_storage fromAddr;

  // Set length of from address structure (in-out parameter)
  socklen_t fromAddrLen = sizeof(fromAddr);
  //  char response[MAX_MESSAGE + 1]; // I/O Buffer
  ssize_t numBytes = recvfrom(sock, response, MAX_MESSAGE, 0, (struct sockaddr *) &fromAddr, &fromAddrLen);
  if (numBytes < 0) {
    err_error(ERR_GET_RESPONSE_FAILED);
    return FAIL;
  }

  // Verify reception from expected source
  if (!compare_sockaddr(servAddr->ai_addr, (struct sockaddr *) &fromAddr)) {
    err_error(ERR_UNKNOWN_RESOURCE);
    return FAIL;
  }

  response[numBytes] = '\0';
  int code;
  sscanf(response, "%d", &code);
  return code;
}

// RESPONSE: STATUS_CODE STATUS DATA(MESSAGE)
int send_response(char *response) {

  // Send response back to the client
  size_t responseLength = strlen(response);
  ssize_t numBytesSent = sendto(sock, response, responseLength, 0, (struct sockaddr *) &clntAddr, sizeof(clntAddr));
  if (numBytesSent < 0) {
    err_error(ERR_SEND_RESPONSE_FAILED);
    return FAIL;
  }

  return SUCCESS;
}

// method: get, post, patch
int send_request(char *method, char *request) {
  requestify(method, request);

  // Length of request
  size_t requestLength = strlen(request);
  if (requestLength > MAX_MESSAGE) {
    err_error(ERR_REQUEST_TOO_LONG);
    return FAIL;
  }

  // Send the string to the server
  ssize_t numBytes = sendto(sock, request, requestLength, 0, servAddr->ai_addr, servAddr->ai_addrlen);
  if (numBytes < 0 || numBytes != requestLength) {
    err_error(ERR_SEND_REQUEST_FAILED);
    return FAIL;
  }

  return SUCCESS;
}
