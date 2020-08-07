#include <stdio.h>
#include <stdlib.h>

#include "socket.h"
#include "game.h"
#include "../helpers.h"

int main(int argc, char **argv) {
    if (argc < 3 || !str_is_numeric(*(argv + 2))) {
        fprintf(stderr, "Usage: client <ip> <port>");
        return 1;
    }

    int port   = strtol(*(argv + 2), NULL, 10);
    int socket = setup_server_socket(*(argv + 1), port);
    init_game(socket);


    return 0;
}
