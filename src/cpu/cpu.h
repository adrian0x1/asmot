#ifndef ASMOT_CPU_H
#define ASMOT_CPU_H

#include <stdbool.h>

#include "../types.h"

typedef struct cpu_times_s {
    f64 user;
    f64 nice;
    f64 system;
    f64 idle;
    f64 iowait;
    f64 irq;
    f64 softirq;
    f64 steal;
    f64 guest;
    f64 guest_nice;
} cpu_times_t;

typedef struct cpu_freq_s {
    f32 current;
    f32 min;
    f32 max;
} cpu_freq_t;

typedef struct cpu_stats_s {
    u64 ctx_switches;
    u64 interrupts;
    u64 soft_interrupts;
} cpu_stats_t;

typedef double loadavg_t;

typedef struct cpu_info_s {
    char arch[8];
    char vendor_str[16];
    char brand_str[64];
    char vendor[8];
    i32 family;
    i32 model;
    u8 num_cores;
    u8 num_logical_cpus;
    u8 total_logical_cpus;
    u32 l1_cache;
    u32 l2_cache;
    u32 l3_cache;
    u32 l4_cache;
    char codename[64];
    char purpose[16];
    i32 clock_by_os;
    i32 measured_clock;
} cpu_info_t;

// TODO: Make it so they all return (-1) on error

/*
    fills out the `cpu_times_t` struct.
    returns `0` if no erros occur otherwise a `positive number` is returned instead.
*/
i32 cpu_times(cpu_times_t* times);

/*
    returns a float representing the total CPU utilization as a percentage
    a negative float is returned if an error occurs
    Note: The interval cannot be less than `1`.
*/
f32 cpu_percent(u32 interval);

/*
    returns the number of cpu cores / threads
*/
u32 cpu_count(bool logical);

/*
    fills out the `cpu_freq_t` struct.
    if an error occurs the frequency is set to `-1.0`
*/
void cpu_freq(cpu_freq_t* freq);

/*
    fills out the `cpu_stats_t` struct.
    if an error occurs the stat is set to `0`
*/
void cpu_stats(cpu_stats_t* stats);

/*
    same as cpu percent but fills the `cpu_stats_t` struct with utilization percentages for each time
    on error it returns a `negative number` otherwise a `0`
*/
i32 cpu_times_percent(cpu_times_t* times);

/*
    Takes in a `loadavg_t []` which will represent the system load over the last 1, 5 and 15 minutes
    The values are representing percentages
    getloadavg() from <stdlib.h> can be used to get the raw values
    on error it returns a `positive number` otherwise a `0`
*/
u32 cpu_loadavg(loadavg_t* load);

/*
    Fills the `cpu_info_t` struct with information about the cpu
    returns `negative number` if an error occurs otherwise it's a `0`
*/
i32 cpu_info(cpu_info_t* cpu);

#endif  // ASMOT_CPU_H
