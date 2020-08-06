#ifndef GAME_H
#define GAME_H
#include <stdbool.h>

#include "socket.h"

#define INVALID_CREATE_ARGS "Error: Invalid args. Correct format \"create [size{3,10}] <name{3,10}> <symbol{1,3}>\""
#define INVALID_JOIN_ARGS "Error: Invalid args. Correct format \"join [code{1000,9999}]\" <name{3,10}> <symbol{1,3}>"
#define INVALID_MOVE_ARGS "Error: Invalid args. Correct format \"move [code{0,board_size}]\""

#define MIN_BOARD_SIZE 3
#define MAX_BOARD_SIZE 10

#define MAX_GAMES 1

typedef enum game_state { AWAITING_JOIN, IN_PROGRESS, FINISHED, STALE } STATE;
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
    struct player *current_player;
    int *board;
};

int handle_command(struct client_socket *socket, char *command);

// Available commands

void create_game(struct game *games[], struct client_socket *socket, char *args);
void join_game(struct game *games[], struct client_socket *socket, char *args);
void make_move(struct game *games[], struct client_socket *socket, char *args);
void reset_game(struct game *games[], struct client_socket *socket, char *args);


void send_board(struct game *game, struct player *player);
void send_game_details(struct game *game);
void send_score_data(struct game *game, struct player *winner);

bool add_game(struct game *games[], struct game *game);
bool is_server_full(struct game *games[]);
void remove_game(struct game *games[], struct game *game);
void disconnect_player(struct game *games[], struct client_socket *socket);
struct game *find_game_by_id(struct game *games[], int id);
struct game *find_game_by_player_fd(struct game *games[], int fd);

#endif
