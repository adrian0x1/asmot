#include <mntent.h>
#include <stdio.h>
#include <string.h>
#include <sys/statvfs.h>
#include <unistd.h>

#include "disk.h"

static bool pseudofs(char* mnt_type) {
    /* List of pseudo file systems */
    char* lpfs[] = {
        "proc", "sysfs", "devtmpfs", "tmpfs", "securityfs", "devpts", "cgroup2", "pstore", "bpf",
        "autofs", "hugetlbfs", "mqueue", "tracefs", "debugfs", "ramfs", "configfs", "fusectl", "binfmt_misc"};
    for (int i = 0; i < sizeof lpfs / sizeof(lpfs[0]); i++)
        if (strcmp(mnt_type, lpfs[i]) == 0) return true;
    return false;
}

i32 disk_partitions(sdiskpart_t* parts, u8 n, bool all) {
    i32 nparts = 0;
    struct statvfs fs;
    struct mntent* entry;
    sdiskpart_t* partition;
    FILE* fp;

    fp = setmntent("/proc/mounts", "r");
    if (fp == NULL) return (-1);

    while ((entry = getmntent(fp)) != NULL) {
        if (!all && pseudofs(entry->mnt_type))
            continue;

        if (statvfs(entry->mnt_dir, &fs) == -1)
            return (-2);

        partition = &parts[nparts++];
        if (nparts >= n)
            return (-3);

        strcpy(partition->device, entry->mnt_fsname);
        strcpy(partition->mountpoint, entry->mnt_dir);
        strcpy(partition->fstype, entry->mnt_type);
        strcpy(partition->opts, entry->mnt_opts);
        partition->maxfile = fs.f_namemax;

        if ((partition->maxpath = pathconf(entry->mnt_dir, _PC_PATH_MAX)) == (-1))
            partition->maxpath = 0;
    }
    endmntent(fp);
    return nparts;
}
