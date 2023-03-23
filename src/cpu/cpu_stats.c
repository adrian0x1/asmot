#include <stdio.h>
#include <string.h>

#include "cpu.h"

void cpu_stats(cpu_stats_t* stats) {
    stats->ctx_switches = 0;
    stats->interrupts = 0;
    stats->soft_interrupts = 0;
    FILE* fp = fopen("/proc/stat", "r");
    if (fp != NULL) {
        char buf[0x50] = {0};
        while (fgets(buf, 0x50, fp) != NULL) {
            if (strstr(buf, "ctxt") != NULL)
                sscanf(buf, "%*s %llu", &stats->ctx_switches);
            else if (strstr(buf, "intr") != NULL)
                sscanf(buf, "%*s %llu", &stats->interrupts);
            else if (strstr(buf, "softirq") != NULL)
                sscanf(buf, "%*s %llu", &stats->soft_interrupts);
            else
                continue;
        }
    }
}
