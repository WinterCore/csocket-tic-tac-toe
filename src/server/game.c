#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/socket.h>

#include "macros.h"
#include "game.h"
#include "game_logic.h"
#include "helpers.h"

static struct game *games[MAX_GAMES] = {NULL};

int handle_command(struct client_socket *socket, char *commandstr) {
    int start = 0, end, l = strlen(commandstr);
    end = read_word(commandstr, &start);
    char *args = malloc(l - end);
    sstrncpy(args, commandstr, l - end, end);
    if (strncmp(commandstr, "create", end - 1) == 0) {
        create_game(games, socket, args);
    } else if (strncmp(commandstr, "join", end - 1) == 0) {
        join_game(games, socket, args);
    } else if (strncmp(commandstr, "disconnect", 10) == 0) {
        disconnect_player(games, socket);
        return 0;
    } else {
        SEND_SOCKET_MESSAGE(socket->fd, "Error: Invalid command");
    }

    return 1;
}


// Format create [board_size<3,10>] <name{3,10}> <symbol{1,3}>
void create_game(struct game *games[], struct client_socket *socket, char *args) {

    struct game *game = find_game_by_player_fd(games, socket->fd);
    if (game != NULL) {
        SEND_SOCKET_MESSAGE(socket->fd, "Error: You're already in a game.");
        return;
    }

    int start = 0, end;
    end = read_word(args, &start);
    char *sizestr = slicestr(args, start, end);
    if (!str_is_numeric(sizestr)) {
        SEND_SOCKET_MESSAGE(socket->fd, INVALID_CREATE_ARGS);
        return;
    }
    int size = atoi(sizestr);
    free(sizestr);
    if (size < 3 || size > 10) {
        SEND_SOCKET_MESSAGE(socket->fd, INVALID_CREATE_ARGS);
        return;
    }

    start = end;
    end = read_word(args, &start);
    if (end - start < 3 || end - start > 20) {
        SEND_SOCKET_MESSAGE(socket->fd, INVALID_CREATE_ARGS);
        return;
    }
    char *name = slicestr(args, start, end);
    start = end;
    end = read_word(args, &start);
    if (end - start < 1 || end - start > 3) {
        SEND_SOCKET_MESSAGE(socket->fd, INVALID_CREATE_ARGS);
        return;
    }

    struct game *new_game = malloc(sizeof(struct game));
    new_game->id              = random_number(); // # TODO: check if the id already exists
    new_game->size            = size;
    new_game->player1         = malloc(sizeof(struct player));
    new_game->player1->socket = socket;
    new_game->player1->name   = name;
    new_game->player1->shape  = slicestr(args, start, end);
    new_game->player1_wins    = 0;
    new_game->player2_wins    = 0;
    new_game->game_state      = AWAITING_JOIN;
    new_game->current_player  = PLAYER1;
    new_game->board           = malloc(size * size * sizeof(PLAYER_NO));
    for (int i = size * size - 1; i >= 0; i -= 1) new_game->board[i] = PLAYER_EMPTY;

    add_game(games, new_game); // TODO: chekc if the games array is full

    char message[50];
    sprintf(message, "CODE: %d\n", new_game->id);
    send(socket->fd, message, strlen(message), 0);
}


// Format create <code{4}> <name{3,10}> <symbol{1,3}>
void join_game(struct game *games[], struct client_socket *socket, char *args) {
    int start = 0, end;
    end = read_word(args, &start);
    char *codestr = slicestr(args, start, end);
    if (end - start != 4 || !str_is_numeric(codestr)) {
        SEND_SOCKET_MESSAGE(socket->fd, INVALID_JOIN_ARGS);
        return;
    }

    int code = atoi(codestr);
    free(codestr);
    struct game *game = find_game_by_player_fd(games, socket->fd);
    if (game != NULL) {
        SEND_SOCKET_MESSAGE(socket->fd, "Error: You're already in a game.");
        return;
    }

    game = find_game_by_id(games, code);
    if (game == NULL || game->game_state != AWAITING_JOIN) {
        SEND_SOCKET_MESSAGE(socket->fd, "Error: The game does not exist.");
        return;
    }

    start = end;
    end = read_word(args, &start);
    if (end < 3 || end > 20) {
        SEND_SOCKET_MESSAGE(socket->fd, INVALID_CREATE_ARGS);
        return;
    }
    char *name = slicestr(args, start, end);
    start = end;
    end = read_word(args, &start);
    if (end - start < 1 || end - start > 3) {
        SEND_SOCKET_MESSAGE(socket->fd, INVALID_CREATE_ARGS);
        return;
    }
    char *shape = slicestr(args, start, end);
    if (strcmp(game->player1->shape, shape) == 0) {
        free(shape);
        SEND_SOCKET_MESSAGE(socket->fd, "Error: You chose the same shape as the other player.");
        return;
    }
    game->player2 = malloc(sizeof(struct player));
    game->player2->socket = socket;
    game->player2->name   = name;
    game->player2->shape  = shape;

    game->game_state = IN_PROGRESS;
    send(game->player2->socket->fd, "SUCCESS\n", 8, 0);

    // Notify each player with the shape of their opponent
    send(game->player2->socket->fd, "OPPONENT_SHAPE ", 16, 0);
    send(game->player2->socket->fd, game->player1->shape, strlen(game->player1->shape), 0);
    send(game->player2->socket->fd, "\n", 1, 0);

    send(game->player1->socket->fd, "OPPONENT_SHAPE ", 16, 0);
    send(game->player1->socket->fd, game->player2->shape, strlen(game->player2->shape), 0);
    send(game->player1->socket->fd, "\n", 1, 0);

    // Notify both players with the board
    char *board = board_to_str(game->board, game->size);
    send(game->player1->socket->fd, board, strlen(board), 0);
    send(game->player2->socket->fd, board, strlen(board), 0);
    free(board);
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

void disconnect_player(struct game *games[], struct client_socket *socket) {
    struct game *game = find_game_by_player_fd(games, socket->fd);
    if (game != NULL) {
        if (game->player1->socket->fd == socket->fd) {
            free(game->player1);
            if (game->player2 != NULL) {
                send(game->player2->socket->fd, "DISCONNECT: Player 1 disconnected.\n", 35, 0);
            }
        } else {
            free(game->player2);
            if (game->player1 != NULL) {
                send(game->player1->socket->fd, "DISCONNECT: Player 2 disconnected.\n", 35, 0);
            }
        }
    }
}

void remove_game(struct game *games[], struct game *game) {
    for (int i = 0; i < MAX_GAMES; i += 1) {
        if (games[i] != NULL && games[i]->id == game->id) {
            if (game->player1 != NULL) {
                free(game->player1->name);
                free(game->player1->shape);
                free(game->player1);
            }
            if (game->player2 != NULL) {
                free(game->player2->name);
                free(game->player2->shape);
                free(game->player2);
            }
            free(game->board);
            break;
        }
    }
}

struct game *find_game_by_id(struct game *games[], int id) {
    for (int i = 0; i < MAX_GAMES; i += 1)
        if (games[i] != NULL && games[i]->id == id)
            return games[i];
    return NULL;
}


struct game *find_game_by_player_fd(struct game *games[], int fd) {
    for (int i = 0; i < MAX_GAMES; i += 1) {
        if (
            games[i] != NULL
            && (
                (games[i]->player1 != NULL && games[i]->player1->socket->fd == fd)
                || (games[i]->player2 != NULL && games[i]->player2->socket->fd == fd)
           )
        ) {
            return games[i];
        }
    }
    return NULL;
}
