#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "request.h"
#include "response.h"
#include "router.h"

#define DEFAULT_PORT 7000
#define BACKLOG 5
// #define BUFSIZE 16384 /* 16K */
#define BUFSIZE (128 * 1024) /* 128K */

typedef struct Server {
    unsigned short port;
    int socket;
    int connection;
    int status;
    struct addrinfo hints;
    struct addrinfo* address;
    struct sockaddr_storage connaddr;
    char buffer[BUFSIZE];
    socklen_t addrlen;
    char ipaddr[INET6_ADDRSTRLEN];
    Router router;
    Request req;
    Response res;
} Server;

void serverInit(Server*);
void serverStatic(Server*, const char*);
void serverGet(Server*, const char*, void (*)(Request*, Response*));
void serverListen(Server*, const char*);

#endif // HTTP_SERVER_H
