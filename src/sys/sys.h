#ifndef ASMOT_SYS_H
#define ASMOT_SYS_H

#include <sys/utsname.h>

#include "../types.h"
#define SYS_BUFLEN 64

typedef struct sysinfo_s {
    char kernel[_UTSNAME_LENGTH];   /* kernel name */
    char release[_UTSNAME_LENGTH];  /* kernel release */
    char version[_UTSNAME_LENGTH];  /* kernel version */
    char hostname[_UTSNAME_LENGTH]; /* nodename */
    char hw_type[_UTSNAME_LENGTH];  /* hardware type */
    char chasis_vendor[SYS_BUFLEN]; /* chassis vendor */
    char vendor[SYS_BUFLEN];        /* system vendor */
    char product_name[SYS_BUFLEN];  /* product name */
} sysinfo_t;

typedef struct user_s {
} user_t;

// TODO: Add documentation
i32 sys_info(sysinfo_t* sys);
// TODO: Implement
i32 boot_time(void);
i32 users(user_t* users);

#endif  // ASMOT_SYS_H