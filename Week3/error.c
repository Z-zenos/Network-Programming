#include <stdio.h>
#include "log.h"
#include "error.h"

#define NUM_OF_ERRORS 20

struct ErrorMessage {
  ErrorCode errorCode;
  char *message;
};

struct ErrorMessage errors[NUM_OF_ERRORS] = {
{ERR_ACCOUNT_NON_ACTIVATED, "Account not activated."},
{ERR_ACCOUNT_BLOCKED, "Account blocked."},
{ERR_ACCOUNT_EXISTED, "Account existed."},
{ERR_ACCOUNT_NOT_FOUND, "Account not found."},
{ERR_ACTIVATION_CODE_INCORRECT, "Activation code incorrect."},
{ERR_INPUT_EMPTY, "Input empty."},
{ERR_INPUT_INVALID, "Input invalid."},
{ERR_OPEN_FILE, "Can't open file."},
{ERR_OPTION, "Bad option."},
{ERR_MEMORY_FULL, "Memory full."},
{ERR_NON_LOG_IN, "You are not logged in."},
{ERR_PASSWORD_INCORRECT, "Password incorrect."},
{ERR_USERNAME_INCORRECT, "Username incorrect."},
{ERR_SERVER_ERROR, "Internal server error."},
{ERR_INVALID_IPv4, "Invalid IP address."},
{ERR_RESOLVE_IP, "Can't not resolve hostname."},
{ERR_INVALID_HOSTNAME, "Invalid hostname."},
{ERR_INVALID_HOMEPAGE_ADDRESS, "Invalid homepage address."},
};

void err_error(ErrorCode err) {
  for (int i = 0; i < NUM_OF_ERRORS; i++)
    if (errors[i].errorCode == err) {
      log_error("%s", errors[i].message);
    }
}