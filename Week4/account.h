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

    // enum int status = [ 1, 0, 2 ];
    int status;
    char homepage[MAX_HOMEPAGE];
    int num_time_wrong_code;
    int num_time_wrong_password;
} Account;

extern Account curr_user;

void signup();
void activate(XOR_LL*);
void signin(XOR_LL*);
Account *search_account(char*);
void search(XOR_LL);
void change_password(XOR_LL*);
void signout(XOR_LL*);
void _set_current_user_(Account);
void _reset_current_user_();
void get_domain();
void get_ipv4();