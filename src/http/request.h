#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H
#include "common.h"

typedef struct Request {
    char* method;
    char* uri;
    char* httpVersion;
    struct Header* headers;
    char* body;
} Request;

void requestInit(Request*);
Request* requestParse(Request*, char*);
void requestDestroy(Request*);

#endif // HTTP_REQUEST_H
