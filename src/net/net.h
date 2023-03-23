#ifndef ASMOT_NET_H
#define ASMOT_NET_H

#include <stdbool.h>

#include "../types.h"

typedef struct snetio_s {
    char interface[16];
    u64 bytes_sent;
    u64 bytes_recv;
    u64 packets_sent;
    u64 packets_recv;
    u64 errin;
    u64 errout;
    u64 dropin;
    u64 dropout;
} snetio_t;

typedef enum kind_e {
    INET,
    INET4,
    INET6,
    TCP,
    TCP4,
    TCP6,
    UDP,
    UDP4,
    UDP6,
    UNIX,
    ALL
} kind_t;

// TODO: Add documentation

i32 net_io_counters(snetio_t *io_counter, bool per_nic);

/* TODO: Implement these functions */
i32 net_connections(kind_t kind);
i32 net_if_addrs(void);
i32 net_if_stats(void);

#endif  // ASMOT_NET_H
