#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "disk.h"
#define SECTOR_SIZE 512

/* ABI: https://www.kernel.org/doc/Documentation/ABI/testing/procfs-diskstats */

static inline bool is_storage_device(const char *dev) {
    char path[PATH_MAX];
    sprintf(path, "/sys/block/%s", dev);
    return access(path, F_OK) == 0 ? true : false;
}

i32 disk_io_counters(sdiskio_t *io_counter, bool per_disk) {
    FILE *fp;
    if ((fp = fopen("/proc/diskstats", "r")) == NULL) {
        return (-1);
    }
    // zero out the (first) structure
    memset(io_counter, 0, sizeof *io_counter);

    u64 reads_completed, reads_merged, sectors_read, time_spent_reading;
    u64 writes_completed, writes_merged, sectors_written, time_spent_writing;
    u64 IOs_in_progress, time_spent_doing_io, weighted_time_spent_doing_io;
    char dev[32] = {0};
    i32 res;
    u32 n = 0;  // number of structures read
    sdiskio_t *org = io_counter;
    /* Ignore the major and minor numbers */
    while ((res = fscanf(fp, "%*u %*u %31s %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %*[^\n]",
                         dev, &reads_completed, &reads_merged, &sectors_read, &time_spent_reading,
                         &writes_completed, &writes_merged, &sectors_written, &time_spent_writing,
                         &IOs_in_progress, &time_spent_doing_io, &weighted_time_spent_doing_io)) == 12) {
        /*****************************************/
        if (!per_disk && !is_storage_device(dev))
            continue;

        else if (per_disk) {
            io_counter = &org[n++];
            memset(io_counter, 0, sizeof *io_counter);
        }

        else
            n = 1;

        io_counter->read_count += reads_completed;
        io_counter->write_count += writes_completed;
        io_counter->read_bytes += sectors_read * SECTOR_SIZE;
        io_counter->write_bytes += sectors_written * SECTOR_SIZE;
        io_counter->read_time += time_spent_reading;
        io_counter->write_time += time_spent_writing;
        io_counter->read_merged_count += reads_merged;
        io_counter->write_merged_count += writes_merged;
    }

    if (ferror(fp)) {
        n = (-2);
    }
    fclose(fp);
    return n;
}
