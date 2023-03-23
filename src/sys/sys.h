#ifndef ASMOT_SYS_H
#define ASMOT_SYS_H

#include <limits.h>
#include <sys/utsname.h>
#include <utmp.h>

#include "../types.h"
#define SYSINFO_BUFLEN 64

typedef struct sysinfo_s {
    char kernel[_UTSNAME_LENGTH];       /* kernel name */
    char release[_UTSNAME_LENGTH];      /* kernel release */
    char version[_UTSNAME_LENGTH];      /* kernel version */
    char hostname[_UTSNAME_LENGTH];     /* nodename */
    char hw_type[_UTSNAME_LENGTH];      /* hardware type */
    char chasis_vendor[SYSINFO_BUFLEN]; /* chassis vendor */
    char vendor[SYSINFO_BUFLEN];        /* system vendor */
    char product_name[SYSINFO_BUFLEN];  /* product name */
} sysinfo_t;

typedef struct suser_s {
    char name[UT_NAMESIZE];     /* username */
    char terminal[UT_LINESIZE]; /* terminal name suffix*/
    char host[UT_HOSTSIZE];     /* hostname for remote login */
    u64 started;                /* time entry was made (in seconds) */
    pid_t pid;                  /* PID of the login process */
} suser_t;

// TODO: Add documentation
i32 sys_info(sysinfo_t* sys);
u64 boot_time(void);
i32 users(suser_t* users, u32 n_users);

#endif  // ASMOT_SYS_H
