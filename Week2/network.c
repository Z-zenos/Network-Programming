#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <regex.h>

#include "network.h"
#include "log.h"
#include "error.h"
#include "constants.h"

bool validate_ip(const char *ip) {
  struct sockaddr_in sa;
  char ip_tmp[BUFFER];
  strcpy(ip_tmp, ip);
  // Convert ip address in xxx.xxx.xxx.xxx format to binary format
  int valid = inet_pton(AF_INET, ip_tmp, &(sa.sin_addr));
  return valid != 0;
}

bool validate_hostname(const char *hostname) {
  char hostname_tmp[BUFFER];
  strcpy(hostname_tmp, hostname);
  regex_t regex;
  int valid;
  valid = regcomp(&regex, "^[a-zA-Z0-9][-a-zA-Z0-9]+[a-zA-Z0-9].[a-z]{2,3}(.[a-z]{2,3})?(.[a-z]{2,3})?$", REG_EXTENDED);
  if (valid == 0) {
    int ret = regexec(&regex, hostname_tmp, (size_t)0, NULL, 0);
    regfree(&regex);
    return !ret ? true : false;
  }
  else {
    return false;
  }
}

// ? Xử lý dns 8.8.8.8 và không dns
bool hostname_to_ip(char *hostname) {
  if(!validate_hostname(hostname))  {
    err_error(ERR_INVALID_HOSTNAME);
    return false;
  }

  struct addrinfo addrConfig, *response, *ipList;
  int errcode;
  char addrStr[100];
  void *ptr;

  // Config type information returned
  memset (&addrConfig, 0, sizeof (addrConfig));
  addrConfig.ai_family = PF_UNSPEC;     // protocol family unspecified
  addrConfig.ai_socktype = SOCK_STREAM; // socket stream for tcp
  addrConfig.ai_flags |= AI_CANONNAME;

  // send request to dns server to get ip address
  errcode = getaddrinfo (hostname, NULL, &addrConfig, &response);
  if (errcode != 0) {
    perror ("getaddrinfo");
    return false;
  }

  ipList = response;
  int first = 0;

  while (ipList) {
    // If is IPv6 then ignore
    if(ipList->ai_family == AF_INET6) break;

    if (ipList->ai_family == AF_INET) {
      // ptr points to ﬁrst byte of a block of memory containing the numeric address to convert.
      ptr = &((struct sockaddr_in *) ipList->ai_addr)->sin_addr; // IPv4 address
    }

    // Convert UPv4 numeric address to readable address and assign to addrStr variable
    inet_ntop (ipList->ai_family, ptr, addrStr, INET_ADDRSTRLEN);
    if(first == 0) {
      log_success("GET / HTTP/1.1 \x1b[1;38;5;47m200\x1b[0m \x1b[4;38;5;226m%s\x1b[0m\r", hostname);
      printf("Official IP:\n\t%s\n", addrStr);
    }
    else {
      if(first == 1) {
        printf("Alias IP: \n");
      }
      printf ("\t%s\n", addrStr);
    }

    ipList = ipList->ai_next;
    first++;
  }

  freeaddrinfo(response);
  if(!first) log_warn("Not found information");
  return true;
}


bool ip_to_hostname(char *ip) {
  if(!validate_ip(ip))  {
    err_error(ERR_INVALID_IPv4);
    return false;
  }

  // Solution 1: Use getnameinfo
  /*
  struct sockaddr_in sa;
  socklen_t len;
  char hostname[NI_MAXHOST];

  memset(&sa, 0, sizeof(struct sockaddr_in));
  sa.sin_family = AF_INET;
  sa.sin_addr.s_addr = inet_addr(ip);
  len = sizeof(struct sockaddr_in);

  if (getnameinfo((struct sockaddr *) &sa, len, hostname, sizeof(hostname),NULL, 0, NI_NAMEREQD)) {
    err_error(ERR_RESOLVE_IP);
    return false;
  }
  else {
    log_success("GET / HTTP/1.1 \x1b[1;38;5;47m200\x1b[0m \x1b[4;38;5;226m%s\x1b[0m\r", ip);
    printf("Official name: %s\n", hostname);

    return true;
  }
   */

  // Solution 2: User gethostbyaddr
  struct hostent *hostname;
  struct in_addr ipAddr;

  inet_aton(ip, &ipAddr);
  hostname = gethostbyaddr(&ipAddr, sizeof(ipAddr), AF_INET);
  if (hostname) {
    int i = 0;
    log_success("GET / HTTP/1.1 \x1b[1;38;5;47m200\x1b[0m \x1b[4;38;5;226m%s\x1b[0m\r", ip);
    printf("Official name:\n\t%s\n", hostname->h_name);
    while (hostname->h_aliases[i] != NULL) {
      if(i == 0) printf("Alias name:");
      printf("\n\t%s", hostname->h_aliases[i++]);
    }
    return true;
  }
  else {
    err_error(ERR_RESOLVE_IP);
    return false;
  }
}