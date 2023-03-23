#include <unistd.h>

#include "cpu.h"

static f64 get_total(cpu_times_t* times) {
    return (times->user + times->nice + times->system + times->idle + times->iowait +
            times->irq + times->softirq + times->steal + times->guest + times->guest_nice);
}

f32 cpu_percent(u32 interval) {
    if (interval < 1) return (-1);
    cpu_times_t times;
    if (cpu_times(&times)) return (-2);
    f64 prev_idle = times.idle, prev_total = get_total(&times);
    sleep(interval);
    if (cpu_times(&times)) return (-3);
    return (f32)((1.0 - (times.idle - prev_idle) / (get_total(&times) - prev_total)) * 100);
}
