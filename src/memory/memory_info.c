#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "memory.h"
#define DMIDECODE_PATH "/usr/bin/dmidecode"
#define SMBIOS_TABLE_ENTRY_POINT "/sys/firmware/dmi/tables/smbios_entry_point"
#define LINE_MAX 128

static inline bool have_dmidecode_access(void) {
    return (access(DMIDECODE_PATH, F_OK) == 0 &&
            access(SMBIOS_TABLE_ENTRY_POINT, R_OK) == 0)
               ? true
               : false;
}

i32 memory_info(memory_info_t* mem) {
    enum read_states {
        SIZE = 1,
        FORM_FACTOR = 1 << 1,
        TYPE = 1 << 2,
        SPEED = 1 << 3,
        MANUFACTURER = 1 << 4,
        ALL = SIZE | FORM_FACTOR | TYPE | SPEED | MANUFACTURER
    };

    if (!have_dmidecode_access()) return 1;
    FILE* stream = popen(DMIDECODE_PATH " -t 17", "r");
    if (stream == NULL)
        return 2;
    char line[LINE_MAX] = {0};
    u8 read = 0;
    while ((fgets(line, LINE_MAX, stream) != NULL) && read ^ ALL) {
        if (line[0] == '\t') line[0] = '#';

        if ((strstr(line, "#Size") != NULL) && !(read & SIZE)) {
            char metric[8] = {0};
            sscanf(line, "%*s %u %s", &mem->size, metric);
            if (strcmp(metric, "GB") == 0) mem->size = mem->size * 1024UL;
            read |= SIZE;
        } else if ((strstr(line, "#Form Factor") != NULL) && !(read & FORM_FACTOR)) {
            sscanf(line, "%*s %*s %s", &mem->form_factor);
            read |= FORM_FACTOR;
        } else if ((strstr(line, "#Type") != NULL) && !(read & TYPE)) {
            sscanf(line, "%*s %s", &mem->type);
            read |= TYPE;
        } else if ((strstr(line, "#Speed") != NULL) && !(read & SPEED)) {
            sscanf(line, "%*s %u %*s", &mem->speed);
            read |= SPEED;
        } else if ((strstr(line, "#Manufacturer") != NULL) && !(read & MANUFACTURER)) {
            sscanf(line, "%*s %s", &mem->manufacturer);
            read |= MANUFACTURER;
        }
        rewind(stream);
    }
    pclose(stream);
    return 0;
}
