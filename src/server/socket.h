#ifndef SOCKET_H
#define SOCKET_H

#include <sys/select.h>

struct client_socket {
    char ip[128];
    int  port;
    int  fd;
};

int setup_server_socket(int port);

void async_handle_connections(int server_socket);

int init_fd_set(fd_set readfds, struct client_socket **client_sockets, int n);

void accept_new_connection(int server_socket, struct client_socket **client_sockets, int n);


#endif
