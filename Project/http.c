#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>

#include "env.h"
#include "http.h"
#include "message.h"

#define NUM_OF_CODE 10

static const int MAX_PENDING = 5; // Maximum outstanding connection requests

int sock;
struct addrinfo *server_addr;
ssize_t numBytesRcvd;

void http_clear(char *method, char *request, char *response) {
  memset(method, 0, 10);
  memset(request, 0, MAX_REQUEST_LENGTH);
  memset(response, 0, MAX_RESPONSE_LENGTH);
}

char *get_socketaddr(const struct sockaddr *address) {
  // Test for address and stream
  if (address == NULL)
    return NULL;

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
      return "unknown";
  }

  // Convert binary to printable address
  if (inet_ntop(address->sa_family, numericAddress, addrBuffer, sizeof(addrBuffer)) == NULL)
    return "unknown";
  else {
    char *addr = (char*)calloc(BUFFER, sizeof(char));
    sprintf(addr, "%s:%u", addrBuffer, port);
    return addr;
  }
}

void requestify(char *method, char *request) {
  /* TEMPLATE: METHOD REQUEST */
  char request_tpl[MAX_LENGTH_MESSAGE];
  sprintf(request_tpl, "%s %s", method, request);
  strcpy(request, request_tpl);
}

void parse_request(char *method, char *request) {
  sscanf(request, "%s %[^\n]s", method, request);
}

int server_init_connect(char *port) {
  // Config the server address structure
  struct addrinfo addrConfig;
  memset(&addrConfig, 0, sizeof(addrConfig)); // Zero out structure
  addrConfig.ai_family    = AF_INET;          // IPv4 address family
  addrConfig.ai_flags     = AI_PASSIVE;       // Accept on any address/port
  addrConfig.ai_socktype  = SOCK_STREAM;      // Only stream socket
  addrConfig.ai_protocol  = IPPROTO_TCP;      // Only TCP socket

  if (getaddrinfo(NULL, port, &addrConfig, &server_addr) != 0) {
    t3_message(T3_SERVER_NOT_FOUND);
    return FAILURE;
  }

  // Create socket for accepting connections
  server_sock = socket(AF_INET, SOCK_STREAM, 0);
  if (server_sock < 0) {
    t3_message(T3_SERVER_CREATE_SOCKET_FAILED);
    return FAILURE;
  }

  // Bind socket to server address
  if (bind(server_sock, server_addr->ai_addr, server_addr->ai_addrlen) < 0) {
    t3_message(T3_SERVER_BIND_SOCKET_FAILED);
    return FAILURE;
  }

  // Listen for connections. Specify the backlog as 5
  if(listen(server_sock, MAX_PENDING) < 0) {
    t3_message(T3_SERVER_LISTEN_CONNECTION_FAILED);
    return FAILURE;
  }

  // Free address list allocated by getaddrinfo()
  freeaddrinfo(server_addr);
  return SUCCESS;
}

int client_init_connect(char *server, char *port) {
  // Tell the system what kind(s) of address info we want - Config address
  struct addrinfo addrConfig;                 // Criteria for address match
  memset(&addrConfig, 0, sizeof(addrConfig)); // Zero out structure
  addrConfig.ai_family   = AF_INET;         // IPv4 address family
  addrConfig.ai_socktype = SOCK_STREAM;       // Only stream sockets
  addrConfig.ai_protocol = IPPROTO_TCP;       // Only TCP protocol

  // Get server information and store data in server_addr
  if (getaddrinfo(server, port, &addrConfig, &server_addr) != 0) {
    t3_message(T3_SERVER_NOT_FOUND);
    return FAILURE;
  }

  // Create a stream TCP socket
  client_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // Socket descriptor for client
  if (client_sock < 0) {
    t3_message(T3_INIT_CONNECT_FAILED);
    return FAILURE;
  }

  // Connect to server
  if(connect(client_sock, (struct sockaddr *)&server_addr, sizeof(*server_addr)) < 0) {
    t3_message(T3_INIT_CONNECT_FAILED);
    return FAILURE;
  }

  return SUCCESS;
}

int get_request(char *method, char *request) {
  // Set Length of client address structure (in-out parameter)
  socklen_t client_addr_len = sizeof(client_addr);
  int connected_sock; // socket connected to client

  // Accept a connection
  connected_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_addr_len);
  if (connected_sock == -1) {
    t3_message(T3_REQUEST_REJECTED);
    return FAILURE;
  }

  // Size of received message DEAL REQUEST FROM CLIENT
  numBytesRcvd = recv(connected_sock, request, MAX_LENGTH_MESSAGE, 0);
  if (numBytesRcvd < 0) {
    t3_message(ERR_GET_REQUEST_FAILUREED);
    return FAILURE;
  }

  request[numBytesRcvd] = '\0';

  // Split method and pure request
  parse_request(method, request);

  char req_time[100];
  time_t now = time(0);
  strftime(req_time, 100, "%Y-%m-%d %H:%M:%S", localtime(&now));
  printf("\x1b[1;38;5;256m%s>\x1b[0m [@\x1b[1;38;5;202m%s\x1b[0m] \x1b[1;38;5;47m%s\x1b[0m \x1b[4m%s\x1b[0m \x1b[1;38;5;226m%ld\x1b[0m\n", req_time, get_socketaddr((struct sockaddr *) &clntAddr), method, request, numBytesRcvd);
  return SUCCESS;
}

int get_response(char *response) {
  struct sockaddr_storage fromAddr;

  //  char response[MAX_MESSAGE + 1]; // I/O Buffer
  ssize_t numBytes = recv(client_sock, response, MAX_LENGTH_MESSAGE, 0);
  if (numBytes < 0) {
    t3_message(ERR_GET_RESPONSE_FAILUREED);
    return FAILURE;
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
  ssize_t numBytesSent = send(connected_sock, response, responseLength, 0);
  if (numBytesSent < 0) {
    t3_message(ERR_SEND_RESPONSE_FAILUREED);
    return FAILURE;
  }

  close(connected_sock);
  return SUCCESS;
}

// method: get, post, patch
int send_request(char *method, char *request) {
  requestify(method, request);

  // Length of request
  size_t requestLength = strlen(request);
  if (requestLength > MAX_LENGTH_MESSAGE) {
    t3_message(T3_REQUEST_TOO_LONG);
    return FAILURE;
  }

  // Send the string to the server
  ssize_t numBytes = send(client_sock, request, requestLength, 0);
  if (numBytes < 0 || numBytes != requestLength) {
    t3_message(T3_SEND_REQUEST_FAILED);
    return FAILURE;
  }

  return SUCCESS;
}
