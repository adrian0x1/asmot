#include <stdio.h>
#include <string.h>

#include "sys.h"
#define PROCFS_STAT "/proc/stat"
#define BTIME "btime"
#define BUFLEN 1024

u64 boot_time(void) {
    FILE* fp = fopen(PROCFS_STAT, "r");
    if (fp == NULL) {
        return 0;
    }
    u64 boot_time = 0;
    char line[BUFLEN], *token;
    while (fgets(line, BUFLEN, fp) != NULL) {
        if (strstr(line, BTIME) != NULL) {
            token = strtok(line, " ");
            if (sscanf((token = strtok(NULL, " ")), "%llu", &boot_time) == 1) {
                break;
            }
        }
    }
    return boot_time;
}
