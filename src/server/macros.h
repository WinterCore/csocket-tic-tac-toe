#ifndef MACROS_H
#define MACROS_H

#define VERSION "0.1.0"

#define PERROR(str) perror(str);\
               exit(EXIT_FAILURE);

#define MAX(x, y) x > y ? x : y

struct client_socket {
    char ip[128];
    int  port;
    int  fd;
};

#endif
