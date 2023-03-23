#ifndef ASMOT_MEMORY_H
#define ASMOT_MEMORY_H
#include "../types.h"

typedef struct svmem_s {
    u64 total;
    u64 available;
    f32 percent;
    u64 used;
    u64 free;
    u64 active;
    u64 inactive;
    u64 buffers;
    u64 cached;
    u64 shared;
    u64 slab;
} svmem_t;

typedef struct sswap_s {
    u64 total;
    u64 used;
    u64 free;
    f32 percent;
    u64 sin;
    u64 sout;
} sswap_t;

typedef struct memory_info_s {
    char manufacturer[16];
    char type[8];
    char form_factor[8];
    u32 size;
    u32 speed;
} memory_info_t;

/*
    fills out the `svmem_t` struct
    on error it returns a `negative value` otherwise a `0`
*/
i32 virtual_memory(svmem_t* mem);

/*
    fills out the `sswap_t` struct
    on error it returns a `negative value` otherwise a `0`
*/
i32 swap_memory(sswap_t* swap);

/*
    fills out the `memory_info_t`
    on error it returs a `positive value` otherwise a `0`
*/
i32 memory_info(memory_info_t* mem);

#endif  // ASMOT_MEMORY_H
