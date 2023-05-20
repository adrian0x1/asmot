#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H
#include "common.h"

typedef struct Response {
    unsigned int statusCode;
    Header* headers;
    size_t length;
    size_t contentLength;
    char* body;
} Response;

void responseInit(Response*);
void responseSendBytes(Response*, const char*, size_t);
void responseSendStatus(Response*, int);
void responseSendFile(Response*, const char*);
void responseSetHeader(Response*, const char*, const char*);
char* responseToBuffer(Response*, char*, size_t);
void responseDestroy(Response*);

#endif // HTTP_RESPONSE_H
