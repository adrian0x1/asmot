#include "router.h"
#include "filelist.h"

void routerInit(Router* router)
{
    router->staticDir = NULL;
    router->staticPaths = NULL;
    router->routes = NULL;
}

void routerStatic(Router* router, const char* dir)
{
    size_t file_count, dirlen;

    file_count = 0;
    dirlen = strlen(dir) + 1;
    router->staticDir = malloc(dirlen);
    strcpy(router->staticDir, dir);

    if ((retrieveFiles(&router->staticPaths, &file_count, dir) < 0)
        || (file_count == 0)) {
        router->staticDir = NULL;
        router->staticPaths = NULL;
    }
}

void routerSetHdl(Router* router, const char* method, const char* uri,
    void (*handler)(Request*, Response*))
{
    Route *entry, *head;
    entry = malloc(sizeof *entry);
    entry->method = malloc(strlen(method) + 1);
    entry->path = malloc(strlen(uri) + 1);

    strcpy(entry->method, method);
    strcpy(entry->path, uri);
    entry->handler = handler;
    entry->next = NULL;

    if (router->routes == NULL) {
        router->routes = entry;
    } else {
        for (head = router->routes; head->next != NULL; head = head->next)
            ;
        head->next = entry;
    }
}

void routerHandle(Router* router, Request* req, Response* res)
{
    /* Only the GET method is currently supported */
    if (strcmp(req->method, "GET") != 0)
        return responseSendStatus(res, HTTP_NOT_IMPLEMENTED);

    /* Check for static match first */
    char path[PATH_MAX];
    sprintf(path, "%s%s", router->staticDir, req->uri);

    for (int i = 0; router->staticPaths[i] != NULL; i++) {
        if (!strcmp(req->method, "GET")
            && !strcmp(router->staticPaths[i], path))
            return responseSendFile(res, router->staticPaths[i]);
    }

    /* Check custom handlers if any are set */
    for (Route* head = router->routes; head != NULL; head = head->next) {
        if (!strcmp(req->method, head->method)
            && !strcmp(req->uri, head->path))
            return head->handler(req, res);
    }

    /* If nothing is found return 404 */
    // responseSendFile(res, "404.html");
    responseSendBytes(res, "<h1>404 Not Found</h1>", 22);
    return responseSendStatus(res, HTTP_NOT_FOUND);
}

void routerDestroy(Router* router)
{
    if (router->staticDir != NULL) {
        free(router->staticDir);
        router->staticDir = NULL;
    }

    if (router->staticPaths != NULL && router->staticPaths[0] != NULL) {
        free(router->staticPaths[0]);
        free(router->staticPaths);
        router->staticPaths = NULL;
    }

    Route *head, *tmp;
    head = router->routes;
    router->routes = NULL;

    while (head != NULL) {
        tmp = head;
        head = head->next;
        free(tmp->method);
        free(tmp->path);
        free(tmp);
    }
}
