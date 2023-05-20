#include <stdio.h>
#include <string.h>
#include <unistd.h>
#define MACHINE_INFO_INTERFACE "/sys/class/dmi/id/"

#include "sys.h"

/* reads a string from a file
    -1 is returned if something went wrong
*/
static int cat(const char *filename, char *buffer, size_t buffer_size) {
    int ret = (-1);
    if (access(filename, F_OK) == 0) {
        FILE *fp = fopen(filename, "r");
        if (fp != NULL) {
            ret = 0;
            if (fscanf(fp, "%s", buffer) == EOF) {
                strcpy(buffer, "Not available");
                ret = (-1);
            }
            fclose(fp);
        } else {
            strcpy(buffer, "Not available");
        }
    }
    return ret;
}

i32 sys_info(sysinfo_t *sys) {
    struct utsname system;
    uname(&system);
    strcpy(sys->kernel, system.sysname);
    strcpy(sys->release, system.release);
    strcpy(sys->version, system.version);
    strcpy(sys->hostname, system.nodename);
    strcpy(sys->hw_type, system.machine);
    enum {
        KEY,
        VALUE,
        KEY_VALUE_PAIR
    };
    char *machine[][KEY_VALUE_PAIR] = {
        {MACHINE_INFO_INTERFACE "chassis_vendor", sys->chasis_vendor},
        {MACHINE_INFO_INTERFACE "sys_vendor", sys->vendor},
        {MACHINE_INFO_INTERFACE "product_name", sys->product_name}};

    for (u8 i = 0; i < sizeof machine / sizeof(machine[0]); i++) {
        cat(machine[i][KEY], machine[i][VALUE], SYSINFO_BUFLEN);
    }
}
