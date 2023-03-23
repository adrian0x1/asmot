#ifndef ASMOT_DISK_H
#define ASMOT_DISK_H

#ifdef __linux__
#include <linux/limits.h>
#endif

#ifdef __APPLE__  // need the declarations for VS Code
#include <sys/syslimits.h>
#endif

#include <stdbool.h>

#include "../types.h"

#define KiB (1ULL << 10)  // kibibyte 2 ^ 10
#define MiB (1ULL << 20)  // mebibyte 2 ^ 20
#define GiB (1ULL << 30)  // gibibyte 2 ^ 30
#define TiB (1ULL << 40)  // tebibyte 2 ^ 40

#define KB 1000ULL           // kilobyte 10 ^ 3
#define MB 1000000ULL        // megabyte 10 ^ 6
#define GB 1000000000ULL     // gigabyte 10 ^ 9
#define TB 1000000000000ULL  // terabyte 10 ^ 12

typedef struct sdiskpart_s {
    char device[NAME_MAX];
    char mountpoint[PATH_MAX];
    char fstype[16];
    char opts[256];
    u32 maxfile;
    u32 maxpath;
} sdiskpart_t;

typedef struct sdiskusage_s {
    u64 total;
    u64 used;
    u64 free;
    f32 percent;
} sdiskusage_t;

typedef struct sdiskio_s {
    u64 read_count;
    u64 write_count;
    u64 read_bytes;
    u64 write_bytes;
    u64 read_time;
    u64 write_time;
    u64 read_merged_count;
    u64 write_merged_count;
    u64 busy_time;  // Note: not implemented yet
} sdiskio_t;

/*
    returns the number of partitions read and a negative value on error
    takes in an array or pointer to n `sdiskpart_t` types
*/
i32 disk_partitions(sdiskpart_t *parts, u8 n, bool all);

/*
 * TODO: Add documentation
 */
i32 disk_usage(sdiskusage_t *du, const char *path);

/*
 */
i32 disk_io_counters(sdiskio_t *io_counter, bool per_disk);

#endif  // ASMOT_DISK_H