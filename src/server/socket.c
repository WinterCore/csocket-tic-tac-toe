#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <arpa/inet.h>

#include "macros.h"
#include "socket.h"

#define SOCKET_BACKLOG 10

int setup_server_socket(int port) {
    int server_socket, opt = 1;
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        PERROR("Failed to create server socket");
    }

    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        PERROR("Failed to set socket option");
    }

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr *) &address, sizeof(address)) < 0) {
        PERROR("Failed to bind server socket");
    }

    if (listen(server_socket, SOCKET_BACKLOG) < 0) {
       PERROR("Failed to listen");
    }
    return server_socket;
}
