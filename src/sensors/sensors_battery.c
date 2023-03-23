#include <stdio.h>
#include <unistd.h>

#include "sensors.h"

/*  ABI Reference
    https://www.kernel.org/doc/Documentation/ABI/testing/sysfs-class-power
*/

/* reads an integer from a file
returns a (-1) if unsuccessful */
static int icat(const char* filename, int* value) {
    *value = (-1);
    if (access(filename, F_OK) == 0) {
        FILE* fp = fopen(filename, "r");
        if (fp != NULL) {
            if (fscanf(fp, "%d", value) == EOF) {
                *value = (-1);
            }
            fclose(fp);
        }
    }
    return *value;
}

int sensors_battery(sbattery_t* battery) {
    char* files[] = {POWER_INTERFACE BATTERY_SUPPLY "/present",
                     POWER_INTERFACE BATTERY_SUPPLY "/capacity",
                     POWER_INTERFACE AC_SUPPLY "/online"};
    enum { PRESENT,
           PERCENT,
           POWER_PLUGGED };
    /* set default values */
    battery->present = false;
    battery->percent = 0;
    battery->power_plugged = false;
    /* try to retrieve the actual values */
    int value, ret = 0;
    for (unsigned i = 0; i < sizeof files / sizeof(files[0]); i++) {
        if (icat(files[i], &value) != (-1)) {
            switch (i) {
                case PRESENT:
                    battery->present = value ? true : false;
                    break;
                case PERCENT:
                    battery->percent = value;
                    break;
                case POWER_PLUGGED:
                    battery->power_plugged = value ? true : false;
                    break;
            }
        } else {
            ret = (-1);
        }
    }

    return ret;
}
