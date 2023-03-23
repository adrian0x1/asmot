#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "cpu/cpu.h"
#include "disk/disk.h"
#include "memory/memory.h"
#include "net/net.h"
#include "sensors/sensors.h"

#define INFO "\x1b[1m"
#define MAX_PARTS 32
#define MB_(n) (f64) n / MB
#define GB_(n) (f64) n / GB
#define NSENSORS 2

enum {
    ERR_PTHREAD = 1,
    ERR_CPUINFO,
    ERR_TIMES,
    ERR_UTILIZATION,
    ERR_FREQ,
    ERR_STATS,
    ERR_LOADAVG,
    ERR_VIRTUAL_MEMORY,
    ERR_SWAP_MEMORY,
    ERR_MEMORY_INFO,
    ERR_DISKPART,
    ERR_DISK_USAGE,
    ERR_DISK_IO_COUNTERS,
    ERR_NETIO_COUNTERS
};

void *get_cpu_times_percent(void *arg) {
    cpu_times_percent((cpu_times_t *)arg);
}

int main(void) {
    pthread_t cpu_times_percent_thread;
    svmem_t vmem;
    sswap_t swap;
    cpu_times_t times;
    cpu_freq_t freq;
    cpu_stats_t stats;
    cpu_times_t ptimes;
    loadavg_t loads[3];
    cpu_info_t cpu;
    memory_info_t mem;
    sdiskpart_t parts[MAX_PARTS];
    sdiskusage_t du;
    sdiskio_t disk_io[MAX_PARTS];
    snetio_t nic_io[3];
    u32 ncounters;  // number of counters read
    i32 nparts;     // number of partitions read
    u32 logical_cores = cpu_count(true), cpu_cores = cpu_count(false);
    temp_sensor_t sensor[NSENSORS];

    if (cpu_info(&cpu)) return ERR_CPUINFO;
    if (memory_info(&mem)) return ERR_MEMORY_INFO;
    if ((nparts = disk_partitions(parts, MAX_PARTS, false)) < 0) return ERR_DISKPART;
    // if ((ncounters = disk_io_counters(disk_io, false)) < 0) return ERR_DISK_IO_COUNTERS;

    u8 running = 1, i = 0;
    i32 pr;

    while (running) {
        // create a thread for cpu_times_percent
        pr = pthread_create(&cpu_times_percent_thread, NULL, &get_cpu_times_percent, (void *)&ptimes);
        if (pr) return ERR_PTHREAD;

        // CPU Times
        if (cpu_times(&times)) return ERR_TIMES;

        printf(
            "\x1b[H\x1b[2J" INFO  // clear screen
            "cpu_times_t\x1b[0m (user=%.0Lf nice=%.0Lf system=%.0Lf idle=%.0Lf iowait=%.0Lf "
            "irq=%.0Lf softirq=%.0Lf steal=%.0Lf guest=%.0Lf guest_nice=%.0Lf)\r\x1b[B\x1b[B",
            times.user, times.nice, times.system, times.idle, times.iowait,
            times.irq, times.softirq, times.steal, times.guest, times.guest_nice);

        // CPU utilization
        // Note: This is a blocking operation of duration `interval`
        f32 utilization = cpu_percent(1);  // in this case `1`
        if (utilization < 0) return ERR_UTILIZATION;

        printf(INFO "cpu_percent_t\x1b[0m (utilization=%.2f%%)\n\n", utilization);

        // CPU cores
        printf(INFO "cpu_count_t\x1b[0m (logical=%d physical=%d)\n\n", logical_cores, cpu_cores);

        // CPU frequencies
        cpu_freq(&freq);
        if (freq.current < 0 || freq.min < 0 || freq.max < 0) return ERR_FREQ;

        printf(INFO "cpu_freq_t\x1b[0m (current=%.02f min=%.02f max=%.02f)\n\n", freq.current, freq.min, freq.max);

        // CPU stats
        cpu_stats(&stats);
        if (!stats.ctx_switches || !stats.interrupts || !stats.soft_interrupts) return ERR_STATS;

        printf(INFO "cpu_stats_t\x1b[0m (ctx_switches=%llu interrupts=%llu soft_interrupts=%llu)\n\n",
               stats.ctx_switches, stats.interrupts, stats.soft_interrupts);

        // CPU load average
        if (cpu_loadavg(loads)) return ERR_LOADAVG;

        printf(INFO "cpu_loadavg_t\x1b[0m (1m=%.2f%% 5m=%.2f%% 15m=%.2f%%)\n\n",
               loads[0], loads[1], loads[2]);

        // CPU percentage times
        pthread_join(cpu_times_percent_thread, NULL);
        printf(
            INFO
            "cpu_times_t[percentage]\x1b[0m (user=%.2Lf%% nice=%.2Lf%% system=%.2Lf%% idle=%.2Lf%% iowait=%.2Lf%% "
            "irq=%.2Lf%% softirq=%.2Lf%% steal=%.2Lf%% guest=%.2Lf%% guest_nice=%.2Lf%%)\n\n",
            ptimes.user, ptimes.nice, ptimes.system, ptimes.idle, ptimes.iowait,
            ptimes.irq, ptimes.softirq, ptimes.steal, ptimes.guest, ptimes.guest_nice);

        // CPU Information
        printf(
            INFO
            "cpu_info_t\x1b[0m (arch=\"%s\" vendor_str=\"%s\" brand_str=\"%s\" "
            "vendor=\"%s\" family=%d model=%d num_cores=%u "
            "num_logical_cpus=%u total_logical_cpus=%u "
            "l1_cache=%uK l2_cache=%uK l3_cache=%uK l4_cache=%uK "
            "codename=\"%s\" purpose=\"%s\" clock_by_os=%d measured_clock=%d)\n\n",
            cpu.arch, cpu.vendor_str, cpu.brand_str,
            cpu.vendor, cpu.family, cpu.model, cpu.num_cores,
            cpu.num_logical_cpus, cpu.total_logical_cpus,
            cpu.l1_cache, cpu.l2_cache, cpu.l3_cache, cpu.l4_cache,
            cpu.codename, cpu.purpose, cpu.clock_by_os, cpu.measured_clock);

        // Memory usage
        if (virtual_memory(&vmem)) return ERR_VIRTUAL_MEMORY;
        if (swap_memory(&swap)) return ERR_SWAP_MEMORY;

        printf(INFO
               "virtual_memory_t\x1b[0m (total=%.1LfM available=%.1LfM percent=%.2f%% used=%.1LfM free=%.1LfM "
               "active=%.1LfM inactive=%.1LfM buffers=%.1LfM cached=%.1LfM shared=%.1LfM slab=%.1LfM)\n\n",
               MB_(vmem.total), MB_(vmem.available), vmem.percent, MB_(vmem.used),
               MB_(vmem.free), MB_(vmem.active), MB_(vmem.inactive), MB_(vmem.buffers),
               MB_(vmem.cached), MB_(vmem.shared), MB_(vmem.slab));

        printf(INFO
               "swap_memory_t\x1b[0m (total=%.1LfM used=%.1LfM "
               "free=%.1LfM percent=%.2f%% sin=%llu sout=%llu)\n\n",
               MB_(swap.total), MB_(swap.used), MB_(swap.free), swap.percent, swap.sin, swap.sout);

        printf(INFO "memory_info_t\x1b[0m (size=%uM form_factor=%s type=%s speed=%u manufacturer=%s)\n\n",
               mem.size, mem.form_factor, mem.type, mem.speed, mem.manufacturer);

        // Disk partitions and usage;
        for (i = 0; i < nparts; i++) {
            printf(INFO "sdiskpart_t\x1b[0m (device='%s' fstype='%s' mountpoint='%s' opts='%s' maxfile=%u maxpath=%u)\n",
                   parts[i].device, parts[i].fstype, parts[i].mountpoint,
                   parts[i].opts, parts[i].maxfile, parts[i].maxpath);

            if (disk_usage(&du, parts[i].mountpoint)) return ERR_DISK_USAGE;

            printf(INFO "sdiskusage_t\x1b[0m (total=%.1LfG used=%.1LfG free=%.1LfG percent=%.1f%%)\n\n",
                   GB_(du.total), GB_(du.used), GB_(du.free), du.percent);
        }
        // usage is a real time metric
        if ((ncounters = disk_io_counters(disk_io, false)) < 0) return ERR_DISK_IO_COUNTERS;
        for (i = 0; i < ncounters; i++) {
            printf(INFO
                   "sdiskio_t\x1b[0m (read_count=%llu write_count=%llu read_bytes=%llu write_bytes=%llu "
                   "read_time=%llu write_time=%llu read_merged_count=%llu write_merged_count=%llu)\n",
                   disk_io[i].read_count, disk_io[i].write_count, disk_io[i].read_bytes, disk_io[i].write_bytes,
                   disk_io[i].read_time, disk_io[i].write_time, disk_io[i].read_merged_count, disk_io[i].write_merged_count);
        }
        // Network I/O
        if ((ncounters = net_io_counters(nic_io, false)) < 0) return ERR_NETIO_COUNTERS;
        for (i = 0; i < ncounters; i++) {
            printf(INFO
                   "\nsnetio_t\x1b[0m (interface=\x1b[33m%s\x1b[0m bytes_sent=%llu bytes_recv=%llu "
                   "packets_sent=%llu packets_recv=%llu errin=%llu errout=%llu dropin=%llu dropout=%llu)\n\n",
                   nic_io[i].interface, nic_io[i].bytes_sent, nic_io[i].bytes_recv, nic_io[i].packets_sent,
                   nic_io[i].packets_recv, nic_io[i].errin, nic_io[i].errout, nic_io[i].dropin, nic_io[i].dropout);
        }
        // Temperature sensors
        if (sensors_temperatures(sensor, 2) < 0) {
            fprintf(stderr, "An error occurred while trying to read the temperatures\n\n");
        } else {
            for (int i = 0; i < 2; i++) {
                printf(INFO "temp_sensor_t\x1b[0m(name=\"%s\" shwtemp_t(label=\"%s\" current=%.1f high=%.1f critical=%.1f))\n",
                       sensor[i].name,
                       sensor[i].temperatures.label,
                       sensor[i].temperatures.current,
                       sensor[i].temperatures.high,
                       sensor[i].temperatures.critical);
            }
        }
    }
    return EXIT_SUCCESS;
}
