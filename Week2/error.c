#include <stdio.h>
#include "log.h"
#include "error.h"

#define NUM_OF_ERRORS 12

struct ErrorMessage {
  ErrorCode errorCode;
  char *message;
};

struct ErrorMessage errors[NUM_OF_ERRORS] = {
{ERR_INPUT_EMPTY, "Input empty."},
{ERR_OPEN_FILE, "Can't open file."},
{ERR_OPTION, "Bad option."},
{ERR_MEMORY_FULL, "Memory full."},
{ERR_INVALID_ARGUMENTS, "Invalid arguments."},
{ERR_INVALID_IPv4, "Invalid IP address."},
{ERR_RESOLVE_IP, "Can't not resolve hostname."},
{ERR_INVALID_HOSTNAME, "Invalid hostname."},
};

void err_error(ErrorCode err) {
  for (int i = 0; i < NUM_OF_ERRORS; i++)
    if (errors[i].errorCode == err) {
      log_error("%s", errors[i].message);
    }
}