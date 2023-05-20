#include <stdlib.h>

#include "cpu.h"

u32 cpu_loadavg(loadavg_t* load)
{
    if (getloadavg(load, 3) == -1)
        return 1;

    u32 cores = cpu_count(true);
    if (cores < 1)
        return 2;

    for (u8 i = 0; i < 3; i++)
        load[i] = (load[i] / cores) * 100;

    return 0;
}
