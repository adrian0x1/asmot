#include <stdbool.h>
#include <stdio.h>
#include <sys/sysinfo.h>

#include "cpu.h"

#define BUFFER_SIZE 256

static u32 get_core_count(void) {
    u32 core_count = 0;
    FILE* fp = fopen("/proc/cpuinfo", "r");

    if (fp == NULL) {
        return core_count;
    }

    char buffer[BUFFER_SIZE];
    while (fgets(buffer, BUFFER_SIZE, fp)) {
        if (sscanf(buffer, "cpu cores\t: %u", &core_count) == 1) {
            break;
        }
    }

    fclose(fp);
    return core_count;
}

u32 cpu_count(bool logical) {
    return logical ? get_nprocs() : get_core_count();
}
