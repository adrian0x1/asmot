#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"

#define MAX_BUFFER_SIZE 256
#define MIN_FREQ_PATH "/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_min_freq"
#define MAX_FREQ_PATH "/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq"
#define CPUINFO_PATH "/proc/cpuinfo"
#define CPU_MHZ "cpu MHz"

void cpu_freq(cpu_freq_t* freq) {
    char buffer[MAX_BUFFER_SIZE];
    FILE* fp = fopen(CPUINFO_PATH, "r");
    freq->current = -1;
    freq->min = -1;
    freq->max = -1;

    if (fp) {
        while (fgets(buffer, MAX_BUFFER_SIZE, fp)) {
            if (strstr(buffer, CPU_MHZ)) {
                freq->current = atof(strchr(buffer, ':') + 2);
                break;
            }
        }
        fclose(fp);
    }

    if ((fp = fopen(MIN_FREQ_PATH, "r"))) {
        fscanf(fp, "%f", &freq->min);
        freq->min /= 1000;
        fclose(fp);
    }

    if ((fp = fopen(MAX_FREQ_PATH, "r"))) {
        fscanf(fp, "%f", &freq->max);
        freq->max /= 1000;
        fclose(fp);
    }
}
