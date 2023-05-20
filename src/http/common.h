#ifndef HTTP_COMMON_H
#define HTTP_COMMON_H

#define HEADER_FIELD_MAX 8192
/* Common HTTP Error Codes */
/* 1xx */
/* 2xx */
#define HTTP_OK 200 /* Success */
/* 3xx */
#define HTTP_MOVED_PERMANENTLY 301 /* 301 Moved Permanently */
/* 4xx */
#define HTTP_BAD_REQUEST 400 /* Bad Request */
#define HTTP_UNAUTHORIZED 401 /* Unauthorized */
#define HTTP_FORBIDDEN 403 /* Forbidden */
#define HTTP_NOT_FOUND 404 /* Not Found */
/* 5xx */
#define HTTP_INTERNAL_SERVER_ERROR 500 /* Internal Server Error */
#define HTTP_NOT_IMPLEMENTED 501 /* Not Implemented */
#define HTTP_BAD_GATEWAY 502 /* Bad Gateway */
#define HTTP_SERVICE_UNAVAILABLE 503 /* Service Unavailable */
#define HTTP_GATEWAY_TIMEOUT 504 /* Gateway Timeout */

#include <ctype.h>
#include <linux/limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define FIELD_SIZEOF(t, f) (sizeof(((t*)0)->f))

typedef struct Header {
    char* name;
    char* value;
    struct Header* next;
} Header;

#endif // HTTP_COMMON_H
