#include <string.h>

#include "sys.h"

i32 users(suser_t* users, u32 n_users) {
    struct utmp* entry;
    suser_t* user;
    u32 count = 0;
    while ((entry = getutent()) != NULL) {
        if (count >= n_users) {
            // TODO:  Add error handling for this
            break;
        }

        if (entry->ut_type == USER_PROCESS) {
            user = &users[count++];
            memcpy(user->name, entry->ut_user, UT_NAMESIZE);     /* username */
            memcpy(user->terminal, entry->ut_line, UT_LINESIZE); /* terminal name suffix */
            memcpy(user->host, entry->ut_host, UT_HOSTSIZE);     /* hostname for login */
            user->pid = entry->ut_pid;                           /* PID of the login process */
            user->started = entry->ut_tv.tv_sec;                 /* time entry was made in seconds */
        }
    }

    endutent();
    return count;
}
