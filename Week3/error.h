#ifndef __ERROR_H__
#define __ERROR_H__

typedef enum {
    ERR_ACCOUNT_NON_ACTIVATED,
    ERR_ACCOUNT_BLOCKED,
    ERR_ACCOUNT_EXISTED,
    ERR_ACCOUNT_NOT_FOUND,
    ERR_ACTIVATION_CODE_INCORRECT,
    ERR_INPUT_EMPTY,
    ERR_INPUT_INVALID,
    ERR_INVALID_HOSTNAME,
    ERR_INVALID_HOMEPAGE_ADDRESS,
    ERR_INVALID_IPv4,
    ERR_MEMORY_FULL,
    ERR_NON_LOG_IN,
    ERR_OPEN_FILE,
    ERR_OPTION,
    ERR_PASSWORD_INCORRECT,
    ERR_RESOLVE_IP,
    ERR_SERVER_ERROR,
    ERR_USERNAME_INCORRECT,
} ErrorCode;

void err_error(ErrorCode);

#endif