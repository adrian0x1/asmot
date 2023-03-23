#include <errno.h>
#include <glob.h>
#include <libgen.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "sensors.h"
#define PATHLEN 128
#define NPATHS 32
#define PATHS_BLOCK (NPATHS * PATHLEN)
#define FILE_PATTERN "hwmon*/temp*_*"
#define CENTOS_PATTERN "hwmon*/device/temp*_*"

/* checks if the os is `CentOS`*/
static inline bool on_centos(void) {
    return access("/etc/centos-release", F_OK) == 0 ? true : false;
}
/* retrieves temperature filepaths */
static int temp_filepaths(char* paths, const char* pattern) {
    glob_t pglob;
    int ret = glob(pattern, 0, NULL, &pglob);

    if (ret == GLOB_NOMATCH) {
        globfree(&pglob);
        return 0;
    } else if (ret == GLOB_NOSPACE || ret == GLOB_ABORTED) {
        globfree(&pglob);
        return (-1);
    }

    for (int i = 0; i < pglob.gl_pathc; i++) {
        strncpy(&paths[i * PATHLEN], pglob.gl_pathv[i], PATHLEN);
        paths[i * PATHLEN + PATHLEN - 1] = '\0';
    }

    ret = pglob.gl_pathc;
    globfree(&pglob);
    return ret;
}
/* removes the latter part of the string beginning from and including the delimiter */
static inline void rm_latter(char* paths, int n_paths, char* delim) {
    char* path;
    char* latter;
    size_t pathlen;
    for (int i = 0; i < n_paths; i++) {
        path = &paths[i * PATHLEN];
        pathlen = strlen(path);
        if (path == NULL || pathlen <= 0) {
            // fprintf(stderr, "Error: Something went wrong while trying to remove the latter part of the path\n");
            continue;
        }
        // check if the path contains the substring
        // if it does zero out the characters starting from latter
        latter = strstr(path, delim);
        if (latter != NULL) {
            memset(latter, 0, strlen(latter));
        }
    }
}
/* removes duplicates */
static inline int rm_dup(char* paths, int n_paths) {
    // TODO: Refactor
    char* ptrs[NPATHS];
    char buffer[PATHS_BLOCK];
    /* copy just the pointers for ease of use */
    for (int i = 0; i < n_paths; i++) {
        ptrs[i] = &paths[i * PATHLEN];
    }

    int i, j = 1, k = 1;
    for (i = 0; i < n_paths; i++) {
        for (j = i + 1, k = j; j < n_paths; j++) {
            if (strcmp(ptrs[i], ptrs[j])) {
                ptrs[k] = ptrs[j];
                k++;
            }
        }
        n_paths -= j - k;
    }
    /* copy the non-duplicate strings into the buffer than copy them back to the array */
    for (int i = 0; i < n_paths; i++) {
        strcpy(&buffer[i * PATHLEN], ptrs[i]);
    }

    memset(paths, 0, n_paths * PATHLEN);
    memcpy(paths, buffer, n_paths * PATHLEN);
    return n_paths;
}

/* appends a word to path */
static void path_append(char* path, const char* word) {
    if (strlen(path) + strlen(word) >= PATHLEN) {
        // fprintf(stderr, "Error: Appending the word to the path is not possible as it's longer than `PATHLEN`\n");
    } else {
        strcat(path, word);
    }
}

/* reads a float */
static inline void fcat(char* path, float* f) {
    FILE* fp = fopen(path, "r");

    if (fp == NULL) {
        *f = 0.0;
        // fprintf(stderr, "Error: Couldn't read float value from path `%s`\n", path);
    } else {
        fscanf(fp, "%f", f);
        *(f) /= 1000.0;
        fclose(fp);
    }
}

/* reads a string */
static inline void cat(char* path, char* name) {
    FILE* fp = fopen(path, "r");

    if (fp == NULL) {
        // fprintf(stderr, "Error: Couldn't read string from path `%s`\n", path);
        strcpy(name, "");
    } else {
        fscanf(fp, "%[^\n]\n", name);
        fclose(fp);
    }
}

/* sets the temperatures for the sensors */
static void set_temps(temp_sensor_t* sensor, char* paths, int n_paths) {
    size_t offset;
    char original_path[PATHLEN];
    char* path;
    for (int i = 0; i < n_paths; i++) {
        sensor = &sensor[i];
        path = &paths[i * PATHLEN];
        strcpy(original_path, path); /* keep a copy of the original path */

        /* NOTE: Some of these reads might fail */
        /* read the current temperature */
        path_append(path, "_input");
        fcat(path, &sensor->temperatures.current);
        /* read the name */
        offset = strlen(dirname(path));
        memset(path + offset, 0, strlen(path) - offset);
        path_append(path, "/name");
        cat(path, sensor->name);
        /* read the high temperature */
        strcpy(path, original_path);
        path_append(path, "_max");
        fcat(path, &sensor->temperatures.high);
        /* read the critical temperature */
        strcpy(path, original_path);
        path_append(path, "_crit");
        fcat(path, &sensor->temperatures.critical);
        /* copy the critical value into high if that failed */
        if (!sensor->temperatures.high && sensor->temperatures.critical) {
            sensor->temperatures.high = sensor->temperatures.critical;
        }
        /* read the label */
        strcpy(path, original_path);
        path_append(path, "_label");
        cat(path, sensor->temperatures.label);
        /* reset the path to the original value */
        strcpy(path, original_path);
    }
}

int sensors_temperatures(temp_sensor_t* sensors, int sensors_count) {
    char paths[PATHS_BLOCK]; /* hwmon filepaths */
    int n_paths;             /* number of paths */
    /* retrieve the temperature filepaths from `/sys/class/hwmon/` */
    n_paths = temp_filepaths(paths,
                             !on_centos() ? TEMP_INTERFACE FILE_PATTERN : TEMP_INTERFACE CENTOS_PATTERN);
    /* remove the filepath endings and than the duplicates */
    rm_latter(paths, n_paths, "_");
    n_paths = rm_dup(paths, n_paths);
    /*
    if there are no paths detected or if the number of paths is bigger than
    the number of sensors a negative value is returned to indicate an error
    */
    if (n_paths <= 0) {
        return (-1);
    } else if (n_paths > sensors_count) {
        return (-2);
    }
    /* set the temperatures */
    set_temps(sensors, paths, n_paths);
    return 0;
}
