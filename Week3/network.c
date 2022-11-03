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

bool validate_domain_name(const char *domain_name) {
  char domain_name_tmp[BUFFER];
  strcpy(domain_name_tmp, domain_name);
  regex_t regex;
  int valid;
  valid = regcomp(&regex, "^[a-zA-Z0-9][-a-zA-Z0-9]+[a-zA-Z0-9].[a-z]{2,3}(.[a-z]{2,3})?(.[a-z]{2,3})?$", REG_EXTENDED);
  if (valid == 0) {
    int ret = regexec(&regex, domain_name_tmp, (size_t)0, NULL, 0);
    regfree(&regex);
    return !ret ? true : false;
  }
  else {
    return false;
  }
}

// ? Xử lý dns 8.8.8.8 và không dns
bool domain_name_to_ip(char *domain_name) {
  if(!validate_domain_name(domain_name))  {
    err_error(ERR_INVALID_DOMAIN_NAME);
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
  errcode = getaddrinfo (domain_name, NULL, &addrConfig, &response);
  if (errcode != 0) {
    perror ("getaddrinfo");
    return false;
  }

  ipList = response;
  int first = 0;

  while (ipList) {
    // Ignore Ipv6
    if(ipList->ai_family == AF_INET6) break;

    if (ipList->ai_family == AF_INET) {
      // ptr points to ﬁrst byte of a block of memory containing the numeric address to convert.
      ptr = &((struct sockaddr_in *) ipList->ai_addr)->sin_addr; // IPv4 address
    }

    // Convert IPv4 numeric address to readable address and assign to addrStr variable
    inet_ntop (ipList->ai_family, ptr, addrStr, INET_ADDRSTRLEN);
    if(first == 0) {
      log_success("GET / HTTP/1.1 \x1b[1;38;5;47m200\x1b[0m \x1b[4;38;5;226m%s\x1b[0m\r", domain_name);
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


bool ip_to_domain_name(char *ip) {
  if(!validate_ip(ip))  {
    err_error(ERR_INVALID_IPv4);
    return false;
  }

  // Solution 1: Use getnameinfo - EFFICIENTLY SOLUTION
  /*
  struct sockaddr_in sa;
  socklen_t len;
  char domain_name[NI_MAXHOST];

  memset(&sa, 0, sizeof(struct sockaddr_in));
  sa.sin_family = AF_INET;
  sa.sin_addr.s_addr = inet_addr(ip);
  len = sizeof(struct sockaddr_in);

  if (getnameinfo((struct sockaddr *) &sa, len, domain_name, sizeof(domain_name),NULL, 0, NI_NAMEREQD)) {
    err_error(ERR_RESOLVE_IP);
    return false;
  }
  else {
    log_success("GET / HTTP/1.1 \x1b[1;38;5;47m200\x1b[0m \x1b[4;38;5;226m%s\x1b[0m\r", ip);
    printf("Official name: %s\n", domain_name);

    return true;
  }
   */

  // Solution 2: Use gethostbyaddr
  // NOTE gethostbyaddr is absolute
  struct hostent *domain_name;
  struct in_addr ipAddr;

  inet_aton(ip, &ipAddr);
  domain_name = gethostbyaddr(&ipAddr, sizeof(ipAddr), AF_INET);
  if (domain_name) {
    int i = 0;
    log_success("GET / HTTP/1.1 \x1b[1;38;5;47m200\x1b[0m \x1b[4;38;5;226m%s\x1b[0m\r", ip);
    printf("Official name:\n\t%s\n", domain_name->h_name);
    while (domain_name->h_aliases[i] != NULL) {
      if(i == 0) printf("Alias name:");
      printf("\n\t%s", domain_name->h_aliases[i++]);
    }
    return true;
  }
  else {
    err_error(ERR_RESOLVE_IP);
    return false;
  }
}