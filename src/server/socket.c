#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "macros.h"
#include "socket.h"
#include "game.h"


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
                int output = handle_socket_data(client);
                if (output == 0) {
                    remove_client_socket(client, client_sockets, MAX_CLIENTS);
                }
            }
        }
    }
}

int handle_socket_data(struct client_socket *client) {
    while (1) {
        int buf_remain = READ_BUFFER_SIZE - client->buffer_size;
        if (buf_remain == 0) {
            char *message = "Error: Buffer is full. Clearing everything";
            send(client->fd, message, strlen(message), 0);
            drain_buffer(client->fd);
            break;
        }

        int nread = read(client->fd, &client->buffer[client->buffer_size], buf_remain);
        if (nread < 0) {
            if (errno == EAGAIN) {
                break;
            } else {
                fprintf(stderr, "Client error %s:%d\n", client->ip, client->port);
                perror("read");
                handle_command(client, "disconnect");
                return 0;
            }
        } else if (nread == 0) {
            printf("Client disconnected %s:%d\n", client->ip, client->port);
            fflush(stdout);
            handle_command(client, "disconnect");
            return 0;
        } else {
            client->buffer_size += nread;
            char *line_start = client->buffer;
            char *line_end;
            while ((line_end = memchr(line_start, '\n', client->buffer_size - (line_start - client->buffer)))) {
                *line_end = '\0';
                char *command = malloc(line_end - line_start + 1);
                memcpy(command, line_start, (line_end - line_start));
                command[line_end - line_start] = '\0';
                send(client->fd, "ACK\n", 4, 0);
                int output = handle_command(client, command);
                free(command);
                if (output == 0) return 0;
                line_start = line_end + 1;
            }
            client->buffer_size -= line_start - client->buffer;
            memmove(client->buffer, line_start, client->buffer_size);

            break;
        }
    }
    return 1;
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
    socket->port        = port;
    socket->fd          = new_socket;
    socket->buffer      = malloc(READ_BUFFER_SIZE);
    socket->buffer_size = 0;
    return socket;
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
        if (client_sockets[i] != NULL && client_sockets[i]->fd == socket->fd) {
            client_sockets[i] = NULL;
            close(socket->fd);
            shutdown(socket->fd, SHUT_RDWR);
            free(socket->buffer);
            free(socket);
            break;
        }
    }
}

