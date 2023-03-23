#ifndef ASMOT_SENSORS_H
#define ASMOT_SENSORS_H

#include <stdbool.h>

#include "../types.h"

#define TEMP_INTERFACE "/sys/class/hwmon/"
#define POWER_INTERFACE "/sys/class/power_supply/"
#define BATTERY_SUPPLY "BAT0"
#define AC_SUPPLY "AC"
#define TEMP_LABEL_MAX 32
#define SENSOR_NAME_MAX 32
#define IBATTERY_STRLEN 32

typedef struct shwtemp_s {
    char label[TEMP_LABEL_MAX];
    f32 current;
    f32 high;
    f32 critical;
} shwtemp_t;

typedef struct temp_sensor_s {
    char name[SENSOR_NAME_MAX];
    shwtemp_t temperatures;
} temp_sensor_t;

typedef struct sbattery_s {
    bool present;
    u32 percent;
    bool power_plugged;
} sbattery_t;

typedef struct ibattery_s {
    char manufacturer[IBATTERY_STRLEN];
    char model_name[IBATTERY_STRLEN];
    char technology[IBATTERY_STRLEN];
    char status[IBATTERY_STRLEN];
} ibattery_t;

// TODO: Write documentation
int sensors_temperatures(temp_sensor_t* sensors, int sensors_count);

// TODO: Must implement
int sensors_fans();
// NOTE: In Progress
int sensors_battery(sbattery_t* battery);
int battery_info(ibattery_t* batinfo);

#endif  // ASMOT_SENSORS_H
