#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "sensors.h"

/*  ABI Reference
    https://www.kernel.org/doc/Documentation/ABI/testing/sysfs-class-power
*/

/* reads a string from a file
    -1 is returned if something went wrong
*/
static int cat(const char *filename, char *buffer, size_t buffer_size) {
    int ret = (-1);
    if (access(filename, F_OK) == 0) {
        FILE *fp = fopen(filename, "r");
        if (fp != NULL) {
            ret = 0;
            if (fscanf(fp, "%s", buffer) == EOF) {
                strcpy(buffer, "Not available");
                ret = (-1);
            }
            fclose(fp);
        } else {
            strcpy(buffer, "Not available");
        }
    }
    return ret;
}

int battery_info(ibattery_t *ibat) {
    const char *files[] = {POWER_INTERFACE BATTERY_SUPPLY "/manufacturer",
                           POWER_INTERFACE BATTERY_SUPPLY "/model_name",
                           POWER_INTERFACE BATTERY_SUPPLY "/technology",
                           POWER_INTERFACE BATTERY_SUPPLY "/status"};
    char *values[] = {ibat->manufacturer, ibat->model_name, ibat->technology, ibat->status};
    size_t files_len = sizeof files / sizeof(files[0]);

    sbattery_t battery;
    sensors_battery(&battery);
    memset(ibat, 0, sizeof(*ibat));
    int ret[files_len];

    if (battery.present) {
        for (int i = 0; i < files_len; i++) {
            ret[i] = cat(files[i], values[i], IBATTERY_STRLEN);
        }
    }

    for (int i = 0; i < files_len; i++) {
        if (ret[i]) {
            return (-1);
        }
    }
    return 0;
}
