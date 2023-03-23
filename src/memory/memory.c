#include "memory.h"

#include <stdio.h>
#include <string.h>

static u64 get(FILE* meminfo, char* metric) {
    char line[64] = {0};
    u64 value = 0;
    while (fgets(line, 64, meminfo) != NULL) {
        if (strstr(line, metric) != NULL) {
            char *word, *line_ptr = line;
            while ((word = strtok(line_ptr, " ")) != NULL) {
                if (word == line_ptr) line_ptr = NULL;
                if (sscanf(word, "%llu", &value) == 1) break;
            }
            break;
        }
    }
    rewind(meminfo);
    return value * 1024;
}

i32 virtual_memory(svmem_t* mem) {
    FILE* meminfo = fopen("/proc/meminfo", "r");
    if (meminfo == NULL)
        return (-1);
    mem->total = get(meminfo, "MemTotal");
    mem->available = get(meminfo, "MemAvailable");
    mem->free = get(meminfo, "MemFree");
    mem->active = get(meminfo, "Active");
    mem->inactive = get(meminfo, "Inactive");
    mem->buffers = get(meminfo, "Buffers");
    mem->cached = get(meminfo, "Cached");
    mem->shared = get(meminfo, "Shmem");
    mem->slab = get(meminfo, "Slab");
    mem->used = mem->total - mem->free - mem->cached - mem->buffers;
    mem->percent = (f32)((mem->total - mem->available) / (f32)mem->total * 100);
    fclose(meminfo);
    return 0;
}

i32 swap_memory(sswap_t* swap) {
    FILE* meminfo = fopen("/proc/meminfo", "r");
    if (meminfo == NULL)
        return (-1);
    swap->total = get(meminfo, "SwapTotal");
    swap->free = get(meminfo, "SwapFree");
    swap->used = swap->total - swap->free;
    swap->percent = (swap->used / (f32)swap->total) * 100;
    fclose(meminfo);
    if ((meminfo = fopen("/proc/vmstat", "r")) == NULL)
        return (-2);
    swap->sin = get(meminfo, "pswpin") * 4;
    swap->sout = get(meminfo, "pswpout") * 4;
    fclose(meminfo);
    return 0;
}