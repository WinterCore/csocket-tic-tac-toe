#include <stdio.h>

#include "macros.h"
#include "socket.h"

#define PORT 8080

int main(int argc, char **argv) {
    int server_socket = setup_server_socket(PORT);
    printf("Server is up and running on port %d\n", PORT);
    fflush(stdout);
    async_handle_connections(server_socket);

    return 0;
}
