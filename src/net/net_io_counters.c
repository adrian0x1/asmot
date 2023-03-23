#include <stdio.h>
#include <string.h>

#include "net.h"

static inline bool starts_with(char *str, const char *prefix) {
    return !strncmp(str, prefix, strlen(prefix)) ? true : false;
}

i32 net_io_counters(snetio_t *io_counter, bool per_nic) {
    FILE *fp;
    if ((fp = fopen("/proc/net/dev", "r")) == NULL) return (-1);

    i32 n_ifread = 0;  // number of interfaces read
    snetio_t *org = io_counter;

    if (!per_nic) {
        memset(io_counter, 0, sizeof *io_counter);
        strcpy(io_counter->interface, "all");
        n_ifread = 1;
    }

    char line[256];
    char *token;
    u64 data[8];
    u8 i, j;

    while (fgets(line, 256, fp) != NULL) {
        if (starts_with(line, "Inter") || starts_with(line, " face"))
            continue;

        token = strtok(line, " ");  // interface name
        if (per_nic) {
            // select the nth counter and initialize it to `0`
            io_counter = &org[n_ifread++];
            memset(io_counter, 0, sizeof *io_counter);
            token[strlen(token) - 1] = '\0';       // remove the colon
            strcpy(io_counter->interface, token);  // add interface name
        }
        // parse the data
        i = 0, j = 16;
        while ((token = strtok(NULL, " ")) != NULL) {
            if (i > 3 && (j >>= 1)) continue;   // skip the rest of the data
            sscanf(token, "%llu", &data[i++]);  // on the receive side
            if (i >= 8) break;
        }

        io_counter->bytes_recv += data[0];
        io_counter->packets_recv += data[1];
        io_counter->errin += data[2];
        io_counter->dropin += data[3];
        io_counter->bytes_sent += data[4];
        io_counter->packets_sent += data[5];
        io_counter->errout += data[6];
        io_counter->dropout += data[7];
    }

    if (ferror(fp)) n_ifread = (-2);
    fclose(fp);
    return n_ifread;
}
