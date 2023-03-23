#include <stdio.h>
#include <unistd.h>

#include "cpu.h"

static f64* cpu_times_at(cpu_times_t *t, u8 i) {
    switch(i) {
        case 0: return &t->user;
        case 1: return &t->nice;
        case 2: return &t->system;
        case 3: return &t->idle;
        case 4: return &t->iowait;
        case 5: return &t->irq;
        case 6: return &t->softirq;
        case 7: return &t->steal;
        case 8: return &t->guest;
        case 9: return &t->guest_nice;
        default: return NULL;
    }
}

i32 cpu_times_percent(cpu_times_t* times) {
    cpu_times_t prev, current;
    if (cpu_times(&prev)) return 1;
    sleep(1);
    if (cpu_times(&current)) return 2;

    u8 i;
    f64 total = 0;
    // calculate difference and add to total
    for (i = 0; i < 10; i++) {
        *cpu_times_at(&current, i) -= *cpu_times_at(&prev, i);
        total += *cpu_times_at(&current, i);
    }

    // set the percentage
    for (i = 0; i < 10; i++)
        *cpu_times_at(times, i) = (*cpu_times_at(&current, i) / total) * 100;
    return 0;
}
