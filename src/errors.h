#ifndef ASMOT_ERRORS_H
#define ASMOT_ERRORS_H
#include <stdio.h>
#include <string.h>

/* global error number */
int a_errno;

/* possible errors */
enum {
    ENONE = 0, /* No errors occurred */
    ETIMES,
    EPERCENT,
    ECOUNT,
    ESTATS,
    EFREQ,
    ELOADAVG,
};

/* equivalent error messages */
const char* err_msg[] = {
    "None",
    "CPU times couldn't be retrieved",
    "CPU utilization percentage couldn't be retrieved",
    "Unable to get CPU count",
    "Unable to get CPU stats",
    "Some or all the CPU frequency stats couldn't be retrieved",
    "Unable to get CPU load averages"};

#ifdef INCLUDE_ERRORS_FNDEF
/* sets `a_errno` to the error code and returnes negative error code */
int error(int err_code) {
    a_errno = err_code;
    return -err_code;
}

/* application perror */
void a_perror(const char* string) {
    if (string != NULL && strlen(string)) {
        fprintf(stderr, "%s: %s\n", string, err_msg[a_errno]);
    }

    else {
        fprintf(stderr, "%s\n", err_msg[a_errno]);
    }
}
#endif  // INCLUDE_ERRORS_FNDEF

#endif  // ASMOT_ERRORS_H
