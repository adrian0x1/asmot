#include <stdio.h>
#include <stdlib.h>

#include "cpu.h"

i32 cpu_times(cpu_times_t* times) {
    FILE* fp;
    i32 r;
    if ((fp = fopen("/proc/stat", "r")) == NULL) return 1;
    r = fscanf(fp, "%*s %Lf %Lf %Lf %Lf %Lf %Lf %Lf %Lf %Lf %Lf",
               &times->user, &times->nice, &times->system, &times->idle, &times->iowait,
               &times->irq, &times->softirq, &times->steal, &times->guest, &times->guest_nice);
    fclose(fp);
    if (r == EOF) return 2;
    return 0;
}
