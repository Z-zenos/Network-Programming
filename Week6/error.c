#include <stdio.h>
#include "log.h"
#include "error.h"

#define NUM_OF_ERRORS 40

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
{ERR_INPUT_INVALID, "Input invalid(no have space, tab)."},
{ERR_OPEN_FILE, "Can't open file."},
{ERR_OPTION, "Bad option."},
{ERR_MEMORY_FULL, "Memory full."},
{ERR_NON_LOG_IN, "You are not logged in."},
{ERR_PASSWORD_INCORRECT, "Password incorrect."},
{ERR_USERNAME_INCORRECT, "Username incorrect."},
{ERR_SERVER_ERROR, "Internal server error."},
{ERR_INVALID_IPv4, "Invalid IP address."},
{ERR_RESOLVE_IP, "Can't not resolve domain name."},
{ERR_INVALID_DOMAIN_NAME, "Invalid domain name."},
{ERR_INVALID_HOMEPAGE_ADDRESS, "Invalid homepage address."},
{ERR_INVALID_SERVER_ARGUMENT, "Invalid server argument."},
{ERR_INVALID_CLIENT_ARGUMENT, "Invalid client argument."},
{ERR_INVALID_PORT, "Invalid port."},
{ERR_SEND_REQUEST_FAILED, "Send request failed."},
{ERR_SEND_RESPONSE_FAILED, "Send response failed."},
{ERR_REQUEST_TOO_LONG, "Request too long."},
{ERR_GET_RESPONSE_FAILED, "Get response failed."},
{ERR_GET_REQUEST_FAILED, "Get request failed."},
{ERR_UNKNOWN_RESOURCE, "Received response from unknown source."},
{ERR_INITIALIZE_SOCKET_FAILED, "Initialize socket failed."},
{ERR_SERVER_NOT_FOUND, "Server not found."},
{ERR_REGISTER_ACCOUNT_FAILED, "Register new account failed."},
{ERR_CLIENT_CONNECT_FAILED, "Client connect failed."},
};

void err_error(ErrorCode err) {
  for (int i = 0; i < NUM_OF_ERRORS; i++)
    if (errors[i].errorCode == err) {
      log_error("%s", errors[i].message);
    }
}