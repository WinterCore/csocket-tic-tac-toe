#include <stdio.h>
#include <stdlib.h>

#include "game.h"
#include "../helpers.h"

int main(int argc, char **argv) {
    if (argc < 3 || !str_is_numeric(*(argv + 2))) {
        fprintf(stderr, "Usage: client <ip> <port>");
        return 1;
    }

    int port   = strtol(*(argv + 2), NULL, 10);
    init_game(*(argv + 1), port);


    return 0;
}
