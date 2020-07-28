#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <string.h>
#include <unistd.h>

#include "macros.h"
#include "socket.h"

#define SOCKET_BACKLOG 10
#define MAX_CLIENTS 50
#define READ_BUFFER_SIZE 50
#define VOID_BUFFER_SIZE 1024

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
        int max_fd = init_fd_set(&readfds, client_sockets, MAX_CLIENTS);
        FD_SET(server_socket, &readfds);
        max_fd = MAX(server_socket, max_fd);

        int activity = select(max_fd + 1, &readfds, NULL, NULL, NULL);

        if (activity < 0) {
            PERROR("Select error");
        }

        if (FD_ISSET(server_socket, &readfds)) {
            struct client_socket *socket = accept_new_connection(server_socket, client_sockets, MAX_CLIENTS);
            add_client_socket(socket, client_sockets, MAX_CLIENTS);
        }

        for (int i = 0; i < MAX_CLIENTS; i += 1) {
            struct client_socket *client = client_sockets[i];
            if (client != NULL && FD_ISSET(client->fd, &readfds)) {
                char *message = read_buffer(client);
                if (message == NULL) {
                    remove_client_socket(client, client_sockets, MAX_CLIENTS);
                } else {
                    printf("- MESSAGE FROM %s:%d: %s", client->ip, client->port, message);
                    fflush(stdout);
                }
                free(message);
            }
        }
    }
}

int init_fd_set(fd_set *readfds, struct client_socket **client_sockets, int n) {
    int max_fd = 0;
    FD_ZERO(readfds);
    for (int i = 0; i < n; i += 1) {
        struct client_socket *client = client_sockets[i];
        if (client != NULL) {
            FD_SET(client->fd, readfds);
            max_fd = MAX(max_fd, client->fd);
        }
    }
    return max_fd;
}


 struct client_socket *accept_new_connection(int server_socket, struct client_socket **client_sockets, int n) {
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    int new_socket = accept(server_socket, (struct sockaddr *) &address, (socklen_t*) &addrlen);
    if (new_socket < 0) {
        perror("Failed to establish connection with client socket\n");
        return NULL;
    }
    char *ip  = inet_ntoa(address.sin_addr);
    int  port = ntohs(address.sin_port);

    printf("New Connection from %s:%d\n", ip, port);
    fflush(stdout);
    char message[100];
    int messagelen = sprintf(message, "Welcome to csocket Tic-Tac-Toe v%s\n", VERSION);

    if (send(new_socket, message, messagelen, 0) != messagelen) {
        fprintf(stderr, "Failed to send welcome message to client\n");
        return NULL;
    }

    struct client_socket *socket = malloc(sizeof(struct client_socket));
    strcpy(socket->ip, ip);
    socket->port = port;
    socket->fd   = new_socket;
    return socket;
}

char *read_buffer(struct client_socket *socket) {
    char *buffer = malloc(READ_BUFFER_SIZE);
    while (1) {
        int nread = read(socket->fd, buffer, READ_BUFFER_SIZE - 1);
        if (nread < 0) {
            fprintf(stderr, "Client error %s:%d\n", socket->ip, socket->port);
            perror("read");
            return NULL;
        } else if (nread == 0) {
            printf("Client disconnected %s:%d\n", socket->ip, socket->port);
            fflush(stdout);
            return NULL;
        } else if (nread == READ_BUFFER_SIZE - 1) {
            drain_buffer(socket->fd);
            buffer[nread] = '\0';
            return buffer;
        } else {
            buffer[nread] = '\0';
            return buffer;
        }
    }

    return NULL;
}

void drain_buffer(int fd) {
    char voidbuffer[VOID_BUFFER_SIZE];
    int val = read(fd, voidbuffer, VOID_BUFFER_SIZE);
    while (1) {
        if (val < VOID_BUFFER_SIZE) {
            break;
        }
    }
}

void add_client_socket(struct client_socket *socket, struct client_socket **client_sockets, int n) {
    for (int i = 0; i < n; i += 1) {
        if (client_sockets[i] == NULL) {
            client_sockets[i] = socket;
            break;
        }
    }
}

void remove_client_socket(struct client_socket *socket, struct client_socket **client_sockets, int n) {
    for (int i = 0; i < n; i += 1) {
        if (client_sockets[i]->fd == socket->fd) {
            client_sockets[i] = NULL;
            close(socket->fd);
            shutdown(socket->fd, SHUT_RDWR);
            free(socket);
            break;
        }
    }
}

