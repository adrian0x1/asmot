#include <stdio.h>
#include <sys/statvfs.h>

#include "disk.h"

i32 disk_usage(sdiskusage_t *du, const char *path) {
    struct statvfs stats;
    if (statvfs(path, &stats) != 0)
        return 1;
    du->total = (u64)stats.f_blocks * stats.f_frsize;
    du->free = (u64)stats.f_bfree * stats.f_frsize;
    du->used = du->total - du->free;
    du->percent = (1.0 * du->used / du->total) * 100.0;
    return 0;
}
