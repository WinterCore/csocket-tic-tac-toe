#include <stdio.h>
#include <stdlib.h>

#include "macros.h"
#include "socket.h"
#include "../helpers.h"

int main(int argc, char **argv) {
    if (argc < 2 || !str_is_numeric(*(argv + 1))) {
        fprintf(stderr, "Usage: server <port>");
        return 1;
    }
    int port = strtol(*(argv + 1), NULL, 10);
    int server_socket = setup_server_socket(port);
    printf("Server is up and running on port %d\n", port);
    fflush(stdout);
    async_handle_connections(server_socket);

    return 0;
}
