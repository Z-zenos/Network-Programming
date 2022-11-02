#include "linkedlist.h"
#include "constants.h"

/**
 * @brief      The structure for user's account
 *
 */

typedef struct _account_ {
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];

    /**
     * @brief   account status
     *          1: active
     *          0: blocked
     *          2: idle
     */
    int status;
    // enum int status = [ 1, 0, 2 ];
} Account;

extern Account curr_user;

void signup(XOR_LL*);
void activate(XOR_LL*);
void signin(XOR_LL*);
Account *search_account(XOR_LL, char*);
void search(XOR_LL);
void change_password(XOR_LL*);
void signout(XOR_LL*);
void _set_current_user_(Account);
void _reset_currect_user();