#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <libcpuid/libcpuid.h>
#include "sysinfo.h"
#define KEY 0
#define VALUE 1
#define KEY_VALUE_PAIR 2
#define MAX_PATH 64
#define LINE_MAX 64
#define MAX_BUFLEN 128
#define DATA_BUFFER_MAX 2048

static void trim_eol(char *, size_t);                  // replaces the new line character in a line with a null terminator
static bool starts_with(const char *, const char *);   // returns true if the string starts with the prefix
static int strip_prefix(const char *, char *, size_t); // removes the given prefix from string
static int rm_char(const char, char *, int, int);      // removes character from string
static int run_dmidecode(char *, size_t);              // runs 'dmidecode -t 17' (memory data) and puts the output into the data buffer
static char *read_line(char *, char *);                // reads a line from a buffer (keeps track of the char offset internaly which must be reset after use)
static int get_os_info(os_info_t *);
static int get_cpu_info(cpu_info_t *);
static int get_machine_info(machine_info_t *);
static int get_mem_info(mem_info_t *);

/*
    sysinfo public functions
*/

sysinfo_t *sysinfo_create(void)
{
    sysinfo_t *sys;

    if (((sys = malloc(sizeof *sys)) == NULL) ||
        ((sys->os = malloc(sizeof *sys->os)) == NULL) ||
        ((sys->cpu = malloc(sizeof *sys->cpu)) == NULL) ||
        ((sys->mach = malloc(sizeof *sys->mach)) == NULL) ||
        ((sys->mem = malloc(sizeof *sys->mem)) == NULL))
    {
        sysinfo_delete(sys);
        return NULL;
    }

    return sys;
}

int sysinfo_retrieve_data(sysinfo_t *sys)
{
    if (get_os_info(sys->os))
        return (-1);
    if (get_cpu_info(sys->cpu))
        return (-1);
    if (get_machine_info(sys->mach))
        return (-1);
    if (get_mem_info(sys->mem))
        return (-1);
    return 0;
}

void sysinfo_delete(sysinfo_t *sys)
{
    if (sys->cpu != NULL)
        free(sys->cpu);
    if (sys->mach != NULL)
        free(sys->mach);
    if (sys->mem != NULL)
        free(sys->mem);
    if (sys->os != NULL)
        free(sys->os);
    if (sys != NULL)
        free(sys);
}

/*
    --- helpers ---
*/

static void trim_eol(char *str, size_t len)
{
    for (int i = 0; i < len; i++)
    {
        if (str[i] == '\n')
        {
            str[i] = '\0';
            break;
        }
    }
}

static bool starts_with(const char *pre, const char *str)
{
    return strncmp(pre, str, strlen(pre)) == 0 ? true : false;
}

static int strip_prefix(const char *pre, char *str, size_t len)
{
    if (len > MAX_BUFLEN)
        return (-1);

    char tmp[MAX_BUFLEN] = {0};

    if (starts_with(pre, str))
    {
        strcpy(tmp, str + strlen(pre));
        memset(str, 0, MAX_BUFLEN);
        strcpy(str, tmp);
    }
    else
        return (-1);

    return 0;
}

static int rm_char(const char c, char *str, int start, int end)
{
    char tmp[MAX_BUFLEN] = {0};
    int i = 0;

    while (start < end)
    {
        if (str[start] != c)
        {
            tmp[i] = str[start];
            i++;
        }

        start++;
    }

    tmp[i] = 0;
    strncpy(str, tmp, strlen(str));
    return 0;
}

static int run_dmidecode(char *data, size_t db_size)
{
    char line[LINE_MAX] = {0};
    int lines_read = 0;
    FILE *stream;

    if ((stream = popen(DMIDECODE_PATH " -t 17", "r")) == NULL)
        return (-1);

    while (fgets(line, LINE_MAX, stream) != NULL)
    {
        if (starts_with("\t", line) && lines_read < (db_size / LINE_MAX))
        {
            rm_char((char)0x09, line, 0, LINE_MAX); // remove the tab character
            rm_char((char)0x20, line, 0, LINE_MAX); // remove the spaces
            strcat(data, line);
            lines_read++;
        }
    }
    pclose(stream);
    return 0;
}

static char *read_line(char *line, char *buf)
{
    static unsigned int offset = 0;

    // reset the offset if either of the args are NULL
    if ((line == NULL) || (buf == NULL))
    {
        offset = 0;
        return NULL;
    }

    int i = 0;
    // read line without the EOL character
    while ((buf[offset] != '\n') && (buf[offset] != '\0') && (i < LINE_MAX))
    {
        line[i] = buf[offset];
        i++;
        offset++;
    }

    line[i] = '\0'; // add the null terminator to the line

    if ((buf[offset] == '\n') && (buf[offset + 1] != '\0'))
    {
        offset++;            // skip over EOL
        return &buf[offset]; // return a valid address
    }

    // reset the offset
    else
    {
        offset = 0;
        return NULL;
    }
}

static int get_os_info(os_info_t *os)
{
    char buf[MAX_BUFLEN] = {0};
    char filepath[MAX_PATH] = {0};
    FILE *fp;

    // Kernel
    char *kernel_files[][KEY_VALUE_PAIR] = {
        {"ostype", os->kernel_type},
        {"osrelease", os->kernel_release},
        {"version", os->kernel_version},
        {"hostname", os->hostname}};

    for (int i = 0; i < sizeof kernel_files / sizeof(char *) / KEY_VALUE_PAIR; i++)
    {
        snprintf(filepath, MAX_PATH, "%s%s", PROC_FILES_DIR, kernel_files[i][KEY]);
        if ((fp = fopen(filepath, "r")) == NULL)
            return (-1);

        while ((fgets(buf, MAX_BUFLEN, fp) != NULL))
        {
            trim_eol(buf, MAX_BUFLEN);
            strcpy(kernel_files[i][VALUE], buf);
            break;
        }

        fclose(fp);
    }

    // Operating System
    if ((fp = fopen(OS_RELEASE_FILE, "r")) == NULL)
        return (-1);

    char *map[][KEY_VALUE_PAIR] = {
        {"NAME=", os->name},
        {"VERSION=", os->full_version},
        {"ID=", os->id},
        {"PRETTY_NAME=", os->full_name},
        {"VERSION_ID=", os->version_id}};

    memset(buf, 0, MAX_BUFLEN);
    while (fgets(buf, MAX_BUFLEN, fp) != NULL)
    {
        for (int i = 0; i < sizeof map / sizeof(char *) / KEY_VALUE_PAIR; i++)
        {
            if (starts_with(map[i][KEY], buf))
            {
                trim_eol(buf, MAX_BUFLEN);
                strip_prefix(map[i][KEY], buf, MAX_BUFLEN);
                starts_with("\"", buf) && rm_char('"', buf, 0, strlen(buf));
                strcpy(map[i][VALUE], buf);
                break;
            }
        }
    }

    fclose(fp);
    return 0;
}

static int get_cpu_info(cpu_info_t *cpu)
{
    {
        if (!cpuid_present())
            return (-1);
    }

    struct cpu_raw_data_t raw_data;
    struct cpu_id_t cpu_data;

    if (cpuid_get_raw_data(&raw_data) < 0)
    {
        return (-2);
    }

    if (cpu_identify(&raw_data, &cpu_data) < 0)
    {
        // printf("Error: %s\n", cpuid_error());
        return (-3);
    }

    // buffer for the int to string conversion
    char data_buffer[64] = {0};

    // set the cpu architecture
    if (cpu_data.architecture == ARCHITECTURE_X86)
        strcpy(cpu->arch, "x86");
    else if (cpu_data.architecture == ARCHITECTURE_ARM)
        strcpy(cpu->arch, "arm");
    else
        strcpy(cpu->arch, "unknown");

    // set the vendor string
    strcpy(cpu->vendor_str, cpu_data.vendor_str);

    // set the brand string
    strcpy(cpu->brand_str, cpu_data.brand_str);

    // set the vendor
    if (cpu_data.vendor == VENDOR_INTEL)
        strcpy(cpu->vendor, "Intel");
    else if (cpu_data.vendor == VENDOR_AMD)
        strcpy(cpu->vendor, "AMD");
    else
        strcpy(cpu->vendor, "Unknown");

    // set the family
    sprintf(data_buffer, "%d", cpu_data.family);
    strcpy(cpu->family, data_buffer);

    // set the cpu model
    sprintf(data_buffer, "%d", cpu_data.model);
    strcpy(cpu->model, data_buffer);

    // set the number of cores
    sprintf(data_buffer, "%d", cpu_data.num_cores);
    strcpy(cpu->num_cores, data_buffer);

    // set the number of logical cpus
    sprintf(data_buffer, "%d", cpu_data.num_logical_cpus);
    strcpy(cpu->num_logical_cpus, data_buffer);

    // set the total logical of cpus
    sprintf(data_buffer, "%d", cpu_data.total_logical_cpus);
    strcpy(cpu->total_logical_cpus, data_buffer);

    // set the L1 cache ammount
    if (cpu_data.l1_data_cache < 0)
        cpu_data.l1_data_cache = 0;
    sprintf(data_buffer, "%d", cpu_data.l1_data_cache);
    strcpy(cpu->l1_cache, data_buffer);

    // set the L2 cache ammount
    if (cpu_data.l2_cache < 0)
        cpu_data.l2_cache = 0;
    sprintf(data_buffer, "%d", cpu_data.l2_cache);
    strcpy(cpu->l2_cache, data_buffer);

    // set the L3 cache ammount
    if (cpu_data.l3_cache < 0)
        cpu_data.l3_cache = 0;
    sprintf(data_buffer, "%d", cpu_data.l3_cache);
    strcpy(cpu->l3_cache, data_buffer);

    // set the L4 cache ammount
    if (cpu_data.l4_cache < 0)
        cpu_data.l4_cache = 0;
    sprintf(data_buffer, "%d", cpu_data.l4_cache);
    strcpy(cpu->l4_cache, data_buffer);

    // set the CPU codename
    strcpy(cpu->cpu_codename, cpu_data.cpu_codename);

    // set the CPU purpose
    cpu->cpu_purpose;
    if (cpu_data.purpose == PURPOSE_GENERAL)
        strcpy(cpu->cpu_purpose, "General");
    else if (cpu_data.purpose == PURPOSE_PERFORMANCE)
        strcpy(cpu->cpu_purpose, "Performance");
    else if (cpu_data.purpose == PURPOSE_EFFICIENCY)
        strcpy(cpu->cpu_purpose, "Efficiency");
    else
        strcpy(cpu->cpu_purpose, "Unknown");

    // set the CPU clock reported by the OS
    sprintf(data_buffer, "%d", cpu_clock_by_os());
    strcpy(cpu->os_cpu_clock, data_buffer);

    // set the CPU clock measured
    sprintf(data_buffer, "%d", cpu_clock_measure(200, 1));
    strcpy(cpu->measured_cpu_clock, data_buffer);

    return 0;
}

static int get_machine_info(machine_info_t *mach)
{
    // read the machine hardware type
    struct utsname hw;
    uname(&hw);
    strcpy(mach->hardware_type, hw.machine);

    // read the other information from sysfs
    char buf[MAX_BUFLEN] = {0};
    FILE *fp;

    char *map[][KEY_VALUE_PAIR] = {
        {MACHINE_INFO_DIR "chassis_vendor", mach->chassis_vendor},
        {MACHINE_INFO_DIR "sys_vendor", mach->sys_vendor},
        {MACHINE_INFO_DIR "product_name", mach->product_name}};

    for (int i = 0; i < sizeof map / sizeof(char *) / KEY_VALUE_PAIR; i++)
    {
        fp = fopen(map[i][KEY], "r");
        while (fgets(buf, MAX_BUFLEN, fp) != NULL)
        {
            trim_eol(buf, MAX_BUFLEN);
            rm_char((char)0x20, buf, 0, MAX_BUFLEN);
            strcpy(map[i][VALUE], buf);
        }
        fclose(fp);
    }
    return 0;
}

static int get_mem_info(mem_info_t *mem)
{
    if ((access(DMIDECODE_PATH, F_OK) == 0) && access(SMBIOS_TABLE_ENTRY_POINT, R_OK) == 0)
    {
        char data[DATA_BUFFER_MAX] = {0};
        char line[LINE_MAX] = {0};

        run_dmidecode(data, DATA_BUFFER_MAX);
        unsigned char read = 0;
        while ((read_line(line, data) != NULL) && read ^ 0xF)
        {
            if (starts_with("Size:", line) && ~read & 1)
            {
                strip_prefix("Size:", line, LINE_MAX);
                strncpy(mem->size, line, 32);
                read |= 1;
            }
            else if (starts_with("Manufacturer:", line) && ~read & 2)
            {
                strip_prefix("Manufacturer:", line, LINE_MAX);
                strncpy(mem->manufacturer, line, 32);
                read |= 2;
            }
            else if (starts_with("Type:", line) && ~read & 4)
            {
                strip_prefix("Type:", line, LINE_MAX);
                strncpy(mem->type, line, 32);
                read |= 4;
            }
            else if (starts_with("Speed:", line) && ~read & 8)
            {
                strip_prefix("Speed:", line, LINE_MAX);
                strncpy(mem->speed, line, 32);
                read |= 8;
            }
        }
        read_line(NULL, NULL); // reset the internal offset in case it didn't have to read all the data
    }

    else
    {
        return (-1);
    }
    return 0;
}
