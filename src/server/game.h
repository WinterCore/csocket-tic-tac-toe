#ifndef GAME_H
#define GAME_H
#include <stdbool.h>

#include "socket.h"

typedef enum game_state { AWAITING_JOIN } STATE;

struct player {
    char *name;
    char *shape;
    struct client_socket *socket;
};

struct game {
    int game_id;
    struct player *player1;
    struct player *player2;
    int player1_wins;
    int player2_wins;
    STATE game_state;
};

int handle_command(struct client_socket *socket, char *command);

void create_game(struct game *games[], struct client_socket *socket, char *args);

bool add_game(struct game *games[], struct game *game);

#endif
