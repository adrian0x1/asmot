#include <libcpuid/libcpuid.h>
#include <string.h>

#include "cpu.h"

i32 cpu_info(cpu_info_t* cpu) {
    if (!cpuid_present()) return (-1);

    struct cpu_raw_data_t raw_data;
    struct cpu_id_t cpu_data;

    if (cpuid_get_raw_data(&raw_data) < 0) return (-2);
    if (cpu_identify(&raw_data, &cpu_data) < 0) return (-3);

    if (cpu_data.architecture == ARCHITECTURE_X86) strcpy(cpu->arch, "x86");
    else if (cpu_data.architecture == ARCHITECTURE_ARM) strcpy(cpu->arch, "arm");
    else strcpy(cpu->arch, "unknown");

    strcpy(cpu->vendor_str, cpu_data.vendor_str);
    strcpy(cpu->brand_str, cpu_data.brand_str);

    if (cpu_data.vendor == VENDOR_INTEL) strcpy(cpu->vendor, "Intel");
    else if (cpu_data.vendor == VENDOR_AMD) strcpy(cpu->vendor, "AMD");
    else strcpy(cpu->vendor, "Other");

    cpu->family = cpu_data.family;
    cpu->model = cpu_data.model;
    cpu->num_cores = (u8)cpu_data.num_cores;
    cpu->num_logical_cpus = (u8)cpu_data.num_logical_cpus;
    cpu->total_logical_cpus = (u8)cpu_data.total_logical_cpus;

    cpu->l1_cache = cpu_data.l1_data_cache < 0 ? 0 : (u32)cpu_data.l1_data_cache;
    cpu->l2_cache = cpu_data.l2_cache < 0 ? 0 : (u32)cpu_data.l2_cache;
    cpu->l3_cache = cpu_data.l3_cache < 0 ? 0 : (u32)cpu_data.l3_cache;
    cpu->l4_cache = cpu_data.l4_cache < 0 ? 0 : (u32)cpu_data.l4_cache;
    strcpy(cpu->codename, cpu_data.cpu_codename);

    switch (cpu_data.purpose) {
        case PURPOSE_GENERAL: strcpy(cpu->purpose, "General"); break;
        case PURPOSE_PERFORMANCE: strcpy(cpu->purpose, "Performance"); break;
        case PURPOSE_EFFICIENCY: strcpy(cpu->purpose, "Efficiency"); break;
        default: strcpy(cpu->purpose, "Unknown"); break;
    }
    cpu->clock_by_os = cpu_clock_by_os();
    cpu->measured_clock = cpu_clock_measure(200, 1);
    return 0;
}
