#ifndef MACROS_H
#define MACROS_H

#define VERSION "0.1.0"

#define PERROR(str) perror(str);\
               exit(EXIT_FAILURE);

#define MAX(x, y) x > y ? x : y

#define SEND_SOCKET_MESSAGE(fd, str) \
        char *message = str "\n"; \
        send(fd, message, strlen(message), 0);

#define SOCKET_BACKLOG 10
#define MAX_CLIENTS 50
#define READ_BUFFER_SIZE 1024
#define VOID_BUFFER_SIZE 1024

struct client_socket {
    char ip[128];
    int  port;
    int  fd;
    char *buffer;
    int  buffer_size;
};

#endif
