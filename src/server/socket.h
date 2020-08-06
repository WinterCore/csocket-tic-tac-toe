#ifndef SOCKET_H
#define SOCKET_H
#include <sys/select.h>

#include "macros.h"

int setup_server_socket(int port);

void async_handle_connections(int server_socket);
int handle_socket_data(struct client_socket *client);

int init_fd_set(fd_set *readfds, struct client_socket **client_sockets, int n);

struct client_socket *accept_new_connection(int server_socket, struct client_socket **client_sockets, int n);

void clear_board(int board[], int size);

void add_client_socket(struct client_socket *socket, struct client_socket **client_sockets, int n);
void remove_client_socket(struct client_socket *socket, struct client_socket **client_sockets, int n);

int read_buffer(struct client_socket *socket);
void drain_buffer(int fd);

#endif
