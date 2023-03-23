#ifndef ARMON_SYSINFO_H
#define ARMON_SYSINFO_H

#define PROC_FILES_DIR "/proc/sys/kernel/"
#define OS_RELEASE_FILE "/etc/os-release"
#define MACHINE_INFO_DIR "/sys/class/dmi/id/"
#define DMIDECODE_PATH "/usr/sbin/dmidecode"
#define SMBIOS_TABLE_ENTRY_POINT "/sys/firmware/dmi/tables/smbios_entry_point"

typedef struct armon_os_info_t
{
    char name[64];
    char id[64];
    char version_id[64];
    char full_name[64];
    char full_version[64];
    char kernel_type[64];
    char kernel_release[64];
    char kernel_version[64];
    char hostname[64];
} os_info_t;

typedef struct armon_cpu_info_t
{
    char arch[64];
    char vendor_str[64];
    char brand_str[64];
    char vendor[64];
    char family[64];
    char model[64];
    char num_cores[64];
    char num_logical_cpus[64];
    char total_logical_cpus[64];
    char l1_cache[64];
    char l2_cache[64];
    char l3_cache[64];
    char l4_cache[64];
    char cpu_codename[64];
    char cpu_purpose[64];
    char os_cpu_clock[64];
    char measured_cpu_clock[64];
} cpu_info_t;

typedef struct armon_machine_info_t
{
    char hardware_type[64];
    char sys_vendor[64];
    char product_name[64];
    char chassis_vendor[64];
} machine_info_t;

typedef struct armon_mem_info_t
{
    char manufacturer[32];
    char type[32];
    char size[32];
    char speed[32];
} mem_info_t;

typedef struct armon_sysinfo_t
{
    machine_info_t *mach;
    os_info_t *os;
    cpu_info_t *cpu;
    mem_info_t *mem;

} sysinfo_t;

sysinfo_t *sysinfo_create(void);
int sysinfo_retrieve_data(sysinfo_t *);
void sysinfo_delete(sysinfo_t *);

#endif /* ARMON_SYSINFO_H */