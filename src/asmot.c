#include <arpa/inet.h> /* inet_ntop function */
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h> /* struct sockaddr_in */
#include <signal.h>     /* need to clean up before exiting on SIGINT */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h> /* TCP syscalls */
#include <time.h>       /* timestamp */
#include <unistd.h>     /* close syscall */

#define PORT 80       /* default port */
#define BACKLOG 2     /* maximum number of pending connections in the fd queue */
#define DATA_MAX 2048 /* size of the data exchange buffer */
#define HTML                               \
    "<!Doctype html>\n"                    \
    "<html>\n"                             \
    "<head>\n"                             \
    "   <title>asmot v0.1.0</title>\n"     \
    "   <style>\n"                         \
    "       * {\n"                         \
    "           margin: 0;\n"              \
    "           padding: 0;\n"             \
    "           font-family: monospace;\n" \
    "         }\n"                         \
    "   </style>\n"                        \
    "</head>\n"                            \
    "<body>\n"                             \
    "   <h1>Hello from asmot!</h1>\n"      \
    "</body>\n"                            \
    "</html>\n"

/* macros for tcp syscalls error checking */
#define chk_fail(tcp_syscall, id)                                \
    if ((tcp_syscall) < 0) {                                     \
        fprintf(stderr, "[ERROR] " id " %s\n", strerror(errno)); \
        exit(-1);                                                \
    }

#define chk_warn(fn, id)                                        \
    if (fn < 0) {                                               \
        fprintf(stderr, "[WARN] " id " %s\n", strerror(errno)); \
    }

/* helper function to handle arguments */
void handle_args(int *argc, char ***argv, unsigned short *port) {
    /* handle incorrect number of arguments */
    if (*argc > 2) {
        fprintf(stderr, "[INFO] Usage: %s PORT\n", (*argv)[0]);
        exit(1);
    }

    /* handle correct number of arguments */
    else if (*argc == 2) {
        *port = (unsigned short)atoi((*argv)[1]);
        if (!*port) {
            fprintf(stderr, "[ERROR] `%s` is not a valid port number.\n", (*argv)[1]);
            exit(2);
        }
    }

    /* if the port is not provided set it to the default port */
    else {
        *port = PORT;
    }
}

/* returns a timestamp as a string */
char *timestamp(const char *fmt) {
    static char buffer[128];
    time_t now = time(NULL);
    strftime(buffer, 128, fmt, localtime(&now));
    return buffer;
}

/* stores socket file descriptors ptrs for use by other functions than main */
enum { SERV_SK,                 /* server socket */
       CONN_SK,                 /* connected socket */
       FD_COUNT };              /* file descriptor count */
static int *sock_fds[FD_COUNT]; /* shared FDs */

/* handle SIGINT*/
void cleanup(int sig) {
    /* check if the conn fd is opened and close it */
    if (fcntl(*(sock_fds[CONN_SK]), F_GETFD) > 0) {
        chk_fail(close(*(sock_fds[CONN_SK])), "<close:conn_sk>");
    }
    /* close the server fd as well before exiting */
    chk_fail(close(*(sock_fds[SERV_SK])), "<close:serv_sk>");

    /* let the user know that the file descriptors have been closed */
    fprintf(stdout, "\rSockets have been closed. Exiting...\n");
    exit(EXIT_SUCCESS);
}

int main(int argc, char **argv) {
    unsigned short port;              /* server port */
    int serv_sk;                      /* server socket */
    int conn_sk;                      /* connected socket */
    struct sockaddr_in serv_addr;     /* server address */
    struct sockaddr_in conn_addr;     /* remote connection address */
    char data[DATA_MAX];              /* data exchange buffer */
    socklen_t addrlen;                /* integer containing the size of conn_addr (in bytes) */
    char ipv4_addr[INET_ADDRSTRLEN];  /* connected socket IPv4 address */
    char ipv6_addr[INET6_ADDRSTRLEN]; /* connected socket IPv6 address */

    /* put the socket addresses into sock_fds */
    sock_fds[SERV_SK] = &serv_sk;
    sock_fds[CONN_SK] = &conn_sk;

    /* handle arguments */
    handle_args(&argc, &argv, &port);

    /* handle SIGINT */
    signal(SIGINT, cleanup);

    /* echo port */
    fprintf(stdout, "[INFO] Starting server on port %d\n", port);

    /* create the server socket */
    chk_fail(serv_sk = socket(AF_INET, SOCK_STREAM, 0), "<socket>");

    /* define the server address information */
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); /* INADDR_ANY doesn't need to be converted really*/
    serv_addr.sin_port = htons(port);

    /* set the socket options */
    chk_fail(setsockopt(serv_sk, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)), "<setsockopt>");

    /* bind the socket to the address */
    chk_fail(bind(serv_sk, (struct sockaddr *)&serv_addr, sizeof(serv_addr)), "<bind>");

    /* start listening for connections */
    chk_fail(listen(serv_sk, BACKLOG), "<listen>");

    /* set the initial address length */
    addrlen = sizeof(conn_addr);

    /* HTTP response */
    char *response =
        /* headers */
        "HTTP/1.1 200 OK\r\n"
        "content-type: text/html; charset=UTF-8\r\n"
        "server: proto\r\n"
        /* data */
        "\r\n" HTML;

    /* handle connections */
    for (;;) {
        /* accept connections */
        chk_fail(conn_sk = accept(serv_sk, (struct sockaddr *)&conn_addr, &addrlen), "<accept>");

        /* receive the message from the connected socket */
        chk_fail(recv(conn_sk, data, DATA_MAX, 0), "<recv>");

        /* Log request */
        fprintf(stdout, "[INFO] Got request from %s ::: %s\n%s",
                inet_ntop(AF_INET, &conn_addr.sin_addr.s_addr, ipv4_addr, addrlen),
                timestamp("%Y-%m-%d %H:%M:%S"), data);

        /* Send the data to client */
        chk_fail(send(conn_sk, response, strlen(response), 0), "<send>");

        /* Close the connection */
        close(conn_sk);
    }

    /* This shouldn't be reached. */
    return EXIT_FAILURE;
}
