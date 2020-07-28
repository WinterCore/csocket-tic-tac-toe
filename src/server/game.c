#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/socket.h>

#include "macros.h"
#include "game.h"
#include "helpers.h"

#define MAX_GAMES 100

static struct game *games[MAX_GAMES] = {NULL};

int handle_command(struct client_socket *socket, char *commandstr) {
    int start = 0, end, l = strlen(commandstr);
    end = read_word(commandstr, &start);
    char *args = malloc(l - end);
    sstrncpy(args, commandstr, l - end, end);
    if (strncmp(commandstr, "create", end - 1) == 0) {
        create_game(games, socket, args);
    } else {
        SEND_SOCKET_MESSAGE(socket->fd, "Invalid command");
    }

    return 1;
}


// Format create <name{3,10}> <symbol{1,3}>
void create_game(struct game *games[], struct client_socket *socket, char *args) {
    int start = 0, end, l = strlen(args);
    end = read_word(args, &start);
    if (end < 3 || end > 20) {
        SEND_SOCKET_MESSAGE(socket->fd, "Invalid args. Correct format \"create <name{3, 10}> <symbol{1,3}>\"");
        return;
    }

    struct game *new_game = malloc(sizeof(struct game));
    new_game->game_id = random_number();
    new_game->player1 = malloc(sizeof(struct player));
    new_game->player1->socket = socket;
    new_game->player1->name   = slicestr(args, start, end);
    new_game->player1_wins    = 0;
    new_game->player2_wins    = 0;
    new_game->game_state      = AWAITING_JOIN;

    start = end;
    end = read_word(args, &start);
    if (end - start < 1 || end - start > 3) {
        SEND_SOCKET_MESSAGE(socket->fd, "Invalid args. Correct format \"create <name{3, 10}> <symbol{1,3}>\"");
        return;
    }
    new_game->player1->shape  = slicestr(args, start, end);

    char message[50];
    sprintf(message, "CODE: %d\n", new_game->game_id);
    send(socket->fd, message, strlen(message), 0);
}




bool add_game(struct game *games[], struct game *game) {
    for (int i = 0; i < MAX_GAMES; i += 1) {
        if (games[i] == NULL) {
            games[i] = game;
            return true;
        }
    }
    return false;
}
