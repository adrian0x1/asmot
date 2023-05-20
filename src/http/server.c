#include "server.h"

void serverInit(Server* server)
{
    memset(server, 0, sizeof *server);
    routerInit(&server->router);
}

void serverStatic(Server* server, const char* path)
{
    routerStatic(&server->router, path);
}

// TODO: Add the other REST methods

void serverGet(Server* server, const char* path, void (*h)(Request*, Response*))
{
    routerSetHdl(&server->router, "GET", path, h);
}

// TODO: Proper error handling
// and make sure all the data is actually sent
void serverListen(Server* server, const char* port)
{
    memset(&server->hints, 0, sizeof server->hints);
    server->hints.ai_family = AF_INET; // IPv4
    server->hints.ai_socktype = SOCK_STREAM; // TCP
    server->hints.ai_flags = AI_PASSIVE;

    if ((server->status = getaddrinfo(NULL, port,
             &server->hints, &server->address))
        != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(server->status));
        exit(-1);
    }

    server->socket = socket(
        server->address->ai_family,
        server->address->ai_socktype,
        server->address->ai_protocol);

    // socket error
    if (server->socket < 0) {
        fprintf(stderr, "socket: %s\n", strerror(errno));
        exit(-2);
    }

    server->status = setsockopt(
        server->socket,
        SOL_SOCKET, SO_REUSEADDR,
        &(int) { 1 }, sizeof(int));

    //  sockopt error
    if (server->status < 0) {
        fprintf(stderr, "sockopt: %s\n", strerror(errno));
        exit(-3);
    }

    server->status = bind(
        server->socket, server->address->ai_addr, server->address->ai_addrlen);

    // bind error
    if (server->status < 0) {
        fprintf(stderr, "bind: %s\n", strerror(errno));
        exit(-4);
    }

    server->status = listen(server->socket, BACKLOG);

    // listen error
    if (server->status < 0) {
        fprintf(stderr, "listen: %s\n", strerror(errno));
        exit(-5);
    }

    server->addrlen = sizeof server->connaddr;
    printf("\x1b[32mListening on port %s\x1b[0m\n", port);

    for (;;) {
        requestInit(&server->req);
        responseInit(&server->res);

        server->connection = accept(server->socket,
            (struct sockaddr*)&server->connaddr, &server->addrlen);

        memset(server->buffer, 0, BUFSIZE);

        recv(server->connection, server->buffer, BUFSIZE, 0);

        requestParse(&server->req, server->buffer);

        routerHandle(&server->router, &server->req, &server->res);

        responseToBuffer(&server->res, server->buffer, BUFSIZE);

        send(server->connection, server->buffer,
            server->res.length + server->res.contentLength, 0);

        close(server->connection);

        requestDestroy(&server->req);
        responseDestroy(&server->res);
    }
    routerDestroy(&server->router);
    freeaddrinfo(server->address);
}
