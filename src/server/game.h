#ifndef GAME_H
#define GAME_H
#include <stdbool.h>

#include "socket.h"

#define INVALID_CREATE_ARGS "Error: Invalid args. Correct format \"create [code{1000,9999}] <name{3,10}> <symbol{1,3}>\""
#define INVALID_JOIN_ARGS "Error: Invalid args. Correct format \"join [size{3,10}]\" <name{3,10}> <symbol{1,3}>"

#define MIN_BOARD_SIZE 3
#define MAX_BOARD_SIZE 10

#define MAX_GAMES 100

typedef enum game_state { AWAITING_JOIN, IN_PROGRESS } STATE;
typedef enum player_no { PLAYER_EMPTY, PLAYER1, PLAYER2 } PLAYER_NO;

struct player {
    char *name;
    char *shape;
    struct client_socket *socket;
};

struct game {
    int id;
    struct player *player1;
    struct player *player2;
    int player1_wins;
    int player2_wins;
    STATE game_state;
    int size;
    PLAYER_NO current_player;
    int *board;
};

int handle_command(struct client_socket *socket, char *command);

// Available commands

void create_game(struct game *games[], struct client_socket *socket, char *args);
void join_game(struct game *games[], struct client_socket *socket, char *args);

bool add_game(struct game *games[], struct game *game);
void remove_game(struct game *games[], struct game *game);
void disconnect_player(struct game *games[], struct client_socket *socket);
struct game *find_game_by_id(struct game *games[], int id);
struct game *find_game_by_player_fd(struct game *games[], int fd);

#endif
