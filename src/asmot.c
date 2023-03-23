#include <stdio.h>
#include <stdlib.h>

#include "cpu/cpu.h"

#define TRY__(fn)                                                                                    \
    {                                                                                                \
        int ret;                                                                                     \
        if ((ret = fn)) {                                                                            \
            fprintf(stderr, "Error at %s:%s:%d returned: %i \n", __FILE__, __func__, __LINE__, ret); \
        }                                                                                            \
    }

int rm1(char* s) {
    return (-1);
}

int main(void) {
    // cpu_info_t cpu;
    TRY__(rm1("abc"));
    TRY__(rm1("bde"));

    return EXIT_SUCCESS;
}
