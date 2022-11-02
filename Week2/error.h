#ifndef __ERROR_H__
#define __ERROR_H__

typedef enum {
    ERR_INPUT_EMPTY,
    ERR_OPEN_FILE,
    ERR_OPTION,
    ERR_MEMORY_FULL,
    ERR_INVALID_ARGUMENTS,
    ERR_INVALID_IPv4,
    ERR_RESOLVE_IP,
    ERR_INVALID_HOSTNAME
} ErrorCode;

void err_error(ErrorCode);

#endif