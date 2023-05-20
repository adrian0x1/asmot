#ifndef HTTP_HELPERS_H
#define HTTP_HELPERS_H
#include "common.h"

char* readField(char*, char*, size_t);
bool isHttpMethod(const char*);
bool isRequestUri(const char*);
bool isHttpVersion(const char*);
bool isRequestLine(char*);
bool isValidRequest(char*);
bool isRequestHeader(char*);
bool httpHasBody(char*);

#endif // HTTP_HELPERS