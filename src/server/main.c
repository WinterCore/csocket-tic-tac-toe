#include <stdio.h>

#include "socket.h"

#define PORT 8080
#define MAX_CLIENTS 50

int main(int argc, char **argv) {
    int server_socket = setup_server_socket(PORT);
    printf("Server is up and running on port %d", PORT);


    return 0;
}
