#ifndef HTTP_ROUTER_H
#define HTTP_ROUTER_H
#include "common.h"
#include "request.h"
#include "response.h"

typedef struct Route {
    char* method;
    char* path;
    void (*handler)(Request*, Response*);
    struct Route* next;
} Route;

typedef struct Router {
    char* staticDir;
    char** staticPaths;
    struct Route* routes;
} Router;

void routerInit(Router*);
void routerStatic(Router*, const char*);
void routerSetHdl(Router*,
    const char*, const char*, void (*)(Request*, Response*));
void routerHandle(Router*, Request*, Response*);
void routerDestroy(Router*);
#endif // HTTP_ROUTER_H
