#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <string.h>

#include "macros.h"
#include "socket.h"

#define SOCKET_BACKLOG 10
#define MAX_CLIENTS 50

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

void async_handle_connections(int server_socket) {
    struct client_socket *client_sockets[MAX_CLIENTS] = {NULL};
    fd_set readfds;
    while (1) {
        int max_fd = init_fd_set(readfds, client_sockets, MAX_CLIENTS);
        FD_SET(server_socket, &readfds);
        max_fd = MAX(server_socket, max_fd);

        int activity = select(max_fd + 1, &readfds, NULL, NULL, NULL);

        if (activity < 0) {
            PERROR("Select error");
        }

        if (FD_ISSET(server_socket, &readfds)) {
            accept_new_connection(server_socket, client_sockets, MAX_CLIENTS);

        }
    }
}

int init_fd_set(fd_set readfds, struct client_socket **client_sockets, int n) {
    int max_fd = 0;
    FD_ZERO(&readfds);
    for (int i = 0; i < n; i += 1) {
        struct client_socket *client = client_sockets[i];
        if (client > 0) {
            FD_SET(client->fd, &readfds);
            max_fd = MAX(max_fd, client->fd);
        }
    }
    return max_fd;
}


void accept_new_connection(int server_socket, struct client_socket **client_sockets, int n) {
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    int new_socket = accept(server_socket, (struct sockaddr *) &address, (socklen_t*) &addrlen);
    if (new_socket < 0) {
        perror("Failed to establish connection with client socket");
    }
    char *ip  = inet_ntoa(address.sin_addr);
    int  port = ntohs(address.sin_port);

    printf("New Connection from %s:%d", ip, port);
    char message[100];
    int messagelen = sprintf(message, "Welcome to csocket Tic-Tac-Toe v%s\n", VERSION);

    if (send(new_socket, message, messagelen, 0) != messagelen) {
        printf("Failed to send welcome message to client");
        return;
    }

    struct client_socket *socket = malloc(sizeof(struct client_socket));
    strcpy(socket->ip, ip);
    socket->port = port;
    socket->fd   = new_socket;
}

