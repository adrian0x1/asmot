#include <dirent.h>
#include <errno.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <unistd.h>

#include "filelist.h"

int getFileCount(const char* dirPath, size_t* fileCount, size_t* reqMem)
{
    DIR* dir;
    struct dirent* entry;
    struct stat sb;
    char fullpath[PATH_MAX];

    if ((dir = opendir(dirPath)) == NULL)
        return (-1);

    while ((entry = readdir(dir)) != NULL) {
        memset(fullpath, 0, strlen(fullpath));
        sprintf(fullpath, "%s/%s", dirPath, entry->d_name);

        if (stat(fullpath, &sb) < 0)
            continue;

        /* DIRECTORIES */
        if (S_ISDIR(sb.st_mode)) {
            if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
                continue;

            getFileCount(fullpath, fileCount, reqMem);
        }

        /* FILES */
        else {
            *fileCount = *fileCount + 1;
            *reqMem = *reqMem + strlen(fullpath) + 1;
        }
    }

    closedir(dir);
    return 0;
}

int setFiles(char* dirPath, char*** files, size_t* fileCount, size_t* offset)
{
    DIR* dir;
    struct dirent* entry;
    struct stat sb;
    char fullpath[PATH_MAX];

    if ((dir = opendir(dirPath)) == NULL)
        return (-1);

    while ((entry = readdir(dir)) != NULL) {
        memset(fullpath, 0, strlen(fullpath));

        sprintf(fullpath, "%s/%s", dirPath, entry->d_name);

        if (stat(fullpath, &sb) < 0)
            continue;

        if (S_ISDIR(sb.st_mode)) {
            if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
                continue;

            setFiles(fullpath, files, fileCount, offset);
        }

        else {
            (*files)[*fileCount] = (*files)[0] + *offset;
            strcpy((*files)[*fileCount], fullpath);
            *offset = *offset + strlen(fullpath) + 1;
            *fileCount = *fileCount + 1;
        }
    }

    closedir(dir);
    return 0;
}

int retrieveFiles(char*** files, size_t* fileCount, const char* dir)
{
    size_t reqmem = 0;
    size_t offset = 0;
    char cwd[PATH_MAX];

    strcpy(cwd, dir);

    if (getFileCount(cwd, fileCount, &reqmem) < 0)
        return (-2);

    *files = malloc((*fileCount + 1) * sizeof(char*));

    if (*files == NULL)
        return (-3);

    (*files)[*fileCount] = NULL;
    (*files)[0] = malloc(reqmem);

    if ((*files)[0] == NULL)
        return (-4);

    reqmem = *fileCount;
    *fileCount = 0;

    if (setFiles(cwd, files, fileCount, &offset) < 0)
        return (-5);

    if (*fileCount != reqmem)
        return (-6);

    return 0;
}
