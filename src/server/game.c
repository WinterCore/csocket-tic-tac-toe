#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/socket.h>

#include "macros.h"
#include "game.h"
#include "game_logic.h"
#include "../helpers.h"

static struct game *games[MAX_GAMES] = {NULL};

int handle_command(struct client_socket *socket, char *commandstr) {
    int output = 1;
    int start = 0, end, l = strlen(commandstr);
    end = read_word(commandstr, &start);
    char *args = malloc(l - end + 1),
         *command = malloc(end + 1);
    sstrncpy(args, commandstr, l - end, end);
    sstrncpy(command, commandstr, end, 0);
    args[l - end] = '\0';
    command[end] = '\0';
    strtolower(command);
    if (strncmp(command, "create", 6) == 0) {
        create_game(games, socket, args);
    } else if (strncmp(command, "join", 4) == 0) {
        join_game(games, socket, args);
    } else if (strncmp(command, "ping", 4) == 0) {
        send(socket->fd, "pong\n", 5, 0);
    } else if (strncmp(command, "move", 4) == 0) {
        make_move(games, socket, args);
    } else if (strncmp(command, "next", 4) == 0) {
        reset_game(games, socket, args);
    } else if (strncmp(command, "disconnect", 10) == 0) {
        struct game *game = find_game_by_player_fd(games, socket->fd);
        disconnect_player(games, socket);
        if (game != NULL) {
            if (game->game_state == AWAITING_JOIN) {
                remove_game(games, game);
            } else {
                game->game_state = AWAITING_JOIN;
            }
        }
        output = 0;
    } else {
        printf("%s:%d sent an invalid command \"%s\"", socket->ip, socket->port, command);
        SEND_SOCKET_MESSAGE(socket->fd, "Error: Invalid command");
    }
    free(args);
    free(command);

    return output;
}

void reset_game(struct game *games[], struct client_socket *socket, char *args) {
    struct game *game = find_game_by_player_fd(games, socket->fd);
    if (game == NULL) {
        SEND_SOCKET_MESSAGE(socket->fd, "Error: You're not in a game.");
        return;
    }

    if (game->game_state != FINISHED) {
        SEND_SOCKET_MESSAGE(socket->fd, "Error: You can only reset finished games.");
        return;
    }

    game->game_state = IN_PROGRESS;
    clear_board(game->board, game->size);
    send(game->player1->socket->fd, "RESET\n", 6, 0);
    send(game->player2->socket->fd, "RESET\n", 6, 0);

    send_board(game, game->player1);
    send_board(game, game->player2);

    send(game->current_player->socket->fd, "YOUR_TURN\n", 10, 0);
}

void make_move(struct game *games[], struct client_socket *socket, char *args) {
    struct game *game = find_game_by_player_fd(games, socket->fd);
    if (game == NULL) {
        SEND_SOCKET_MESSAGE(socket->fd, "Error: You're not in a game.");
        return;
    }
    if (game->game_state != IN_PROGRESS) {
        SEND_SOCKET_MESSAGE(socket->fd, "Error: The current game is not in progress.");
        return;
    }

    if (game->current_player->socket->fd != socket->fd) {
        SEND_SOCKET_MESSAGE(socket->fd, "Error: It's not your turn.");
        return;
    }

    int start = 0, end;
    end = read_word(args, &start);
    char *posstr = slicestr(args, start, end);
    if (!str_is_numeric(posstr)) {
        free(posstr);
        SEND_SOCKET_MESSAGE(socket->fd, INVALID_CREATE_ARGS);
        return;
    }
    int pos = strtol(posstr, NULL, 10);
    free(posstr);

    if (pos < 0 || pos >= game->size * game->size) {
        SEND_SOCKET_MESSAGE(socket->fd, INVALID_MOVE_ARGS);
        return;
    }

    if (game->board[pos] != PLAYER_EMPTY) {
        SEND_SOCKET_MESSAGE(socket->fd, "Error: The cell you chose is already filled.");
        return;
    }

    if (game->player1->socket->fd == socket->fd) {
        game->board[pos] = PLAYER1;
        game->current_player = game->player2;
    } else {
        game->board[pos] = PLAYER2;
        game->current_player = game->player1;
    }

    // Notify both players with the board
    send_board(game, game->player1);
    send_board(game, game->player2);

    if (
        check_diagonal(game->board, game->size)
        || check_horizontal(game->board, game->size)
        || check_vertical(game->board, game->size)
    ) {
        game->game_state = FINISHED;
        if (game->current_player->socket->fd == game->player2->socket->fd) {
            game->player1_wins += 1;
            send(game->player1->socket->fd, "WIN\n", 4, 0);
            send(game->player2->socket->fd, "LOSE\n", 5, 0);
        } else {
            game->player2_wins += 1;
            send(game->player2->socket->fd, "WIN\n", 4, 0);
            send(game->player1->socket->fd, "LOSE\n", 5, 0);
        }
        send_score_data(game, game->player1);
        send_score_data(game, game->player2);
        game->game_state = FINISHED;
    } else if (is_board_full(game->board, game->size)) {
        game->game_state = FINISHED;
        send(game->player1->socket->fd, "DRAW\n", 5, 0);
        send(game->player2->socket->fd, "DRAW\n", 5, 0);
    } else {
        send(game->current_player->socket->fd, "YOUR_TURN\n", 10, 0);
    }
}



// Format create [board_size<3,10>] <name{3,10}> <symbol{1,3}>
void create_game(struct game *games[], struct client_socket *socket, char *args) {
    struct game *game = find_game_by_player_fd(games, socket->fd);
    if (game != NULL) {
        SEND_SOCKET_MESSAGE(socket->fd, "Error: You're already in a game.");
        return;
    }

    if (is_server_full(games)) {
        SEND_SOCKET_MESSAGE(socket->fd, "Error: The server is currently full. Please try again later");
        return;
    }

    int start = 0, end;
    end = read_word(args, &start);
    char *sizestr = slicestr(args, start, end);
    if (!str_is_numeric(sizestr)) {
        SEND_SOCKET_MESSAGE(socket->fd, INVALID_CREATE_ARGS);
        return;
    }
    int size = strtol(sizestr, NULL, 10);
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
    int game_id = random_number();
    while (find_game_by_id(games, game_id) != NULL) {
        game_id = random_number();
    }
    new_game->id              = game_id;
    new_game->size            = size;
    new_game->player1         = malloc(sizeof(struct player));
    new_game->player1->socket = socket;
    new_game->player1->name   = name;
    new_game->player1->shape  = slicestr(args, start, end);
    new_game->player1_wins    = 0;
    new_game->player2_wins    = 0;
    new_game->game_state      = AWAITING_JOIN;
    new_game->current_player  = new_game->player1;
    new_game->board           = malloc(size * size * sizeof(PLAYER_NO));
    clear_board(new_game->board, size);

    add_game(games, new_game);

    char message[50];
    sprintf(message, "CODE: %d\n", new_game->id);
    send(socket->fd, message, strlen(message), 0);
}

void clear_board(int board[], int size) {
    for (int i = size * size - 1; i >= 0; i -= 1)
        board[i] = PLAYER_EMPTY;
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

    int code = strtol(codestr, NULL, 10);
    free(codestr);
    struct game *game = find_game_by_player_fd(games, socket->fd);
    if (game != NULL) {
        SEND_SOCKET_MESSAGE(socket->fd, "Error: You're already in a game.");
        return;
    }

    game = find_game_by_id(games, code);
    if (game == NULL || game->game_state != AWAITING_JOIN) {
        SEND_SOCKET_MESSAGE(socket->fd, "NOT_FOUND_ERR Error: The game does not exist.");
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
        free(name);
        SEND_SOCKET_MESSAGE(socket->fd, INVALID_CREATE_ARGS);
        return;
    }
    char *shape = slicestr(args, start, end);
    if (strcmp(game->player1->shape, shape) == 0) {
        free(name);
        free(shape);
        SEND_SOCKET_MESSAGE(socket->fd, "SAME_SHAPE_ERR Error: You chose the same shape as the other player.");
        return;
    }
    send(socket->fd, "SUCCESS\n", 8, 0);

    struct player *player;

    if (game->player2 == NULL) {
        game->player2 = malloc(sizeof(struct player));
        player = game->player2;
    } else {
        game->player1 = malloc(sizeof(struct player));
        player = game->player1;
    }

    player->socket = socket;
    player->name   = name;
    player->shape  = shape;
    if (game->current_player == NULL)
        game->current_player = player;

    game->game_state = IN_PROGRESS;

    send_game_details(game);
    send_score_data(game, game->player1);
    send_score_data(game, game->player2);

    send(game->player1->socket->fd, "GAME_START\n", 12, 0);
    send(game->player2->socket->fd, "GAME_START\n", 12, 0);

    send_board(game, game->player1);
    send_board(game, game->player2);

    send(game->current_player->socket->fd, "YOUR_TURN\n", 10, 0);
}

void send_game_details(struct game *game) {
    char msg[200];

    char *format_str = "OPPONENT_INDICATOR %d\n"
                       "YOUR_INDICATOR %d\n"
                       "GAME_SIZE %d\n"
                       "OPPONENT_SHAPE %s\n"
                       "OPPONENT_NAME %s\n";

    sprintf(
               msg,
               format_str,
               PLAYER1,
               PLAYER2,
               game->size,
               game->player1->shape,
               game->player1->name
           );
    send(game->player2->socket->fd, msg, strlen(msg), 0);

    sprintf(
               msg,
               format_str,
               PLAYER2,
               PLAYER1,
               game->size,
               game->player2->shape,
               game->player2->name
           );
    send(game->player1->socket->fd, msg, strlen(msg), 0);
}

void send_board(struct game *game, struct player *player) {
    char *board = board_to_str(game->board, game->size);
    char msg[50];
    sprintf(msg, "BOARD %s\n", board);
    send(player->socket->fd, msg, strlen(msg), 0);
    free(board);
}

void send_score_data(struct game *game, struct player *player) {
    char msg[50];
    if (player == game->player1)
        sprintf(msg, "SCORE %d:%d\n", game->player1_wins, game->player2_wins);
    else
        sprintf(msg, "SCORE %d:%d\n", game->player2_wins, game->player1_wins);
    send(player->socket->fd, msg, strlen(msg), 0);
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

bool is_server_full(struct game *games[]) {
    for (int i = 0; i < MAX_GAMES; i += 1) {
        if (games[i] == NULL) {
            return false;
        }
    }
    return true;
}

void disconnect_player(struct game *games[], struct client_socket *socket) {
    struct game *game = find_game_by_player_fd(games, socket->fd);
    if (game != NULL) {
        if (game->player1 != NULL && game->player1->socket->fd == socket->fd) {
            free(game->player1->name);
            free(game->player1->shape);
            free(game->player1);
            if (game->current_player == game->player1) {
                game->current_player = NULL;
            }
            game->player1 = NULL;
            if (game->player2 != NULL) {
                send(game->player2->socket->fd, "DISCONNECT Player 1 disconnected.\n", 35, 0);
            }
        } else if (game->player2 != NULL && game->player2->socket->fd == socket->fd) {
            free(game->player2->name);
            free(game->player2->shape);
            free(game->player2);
            if (game->current_player == game->player2) {
                game->current_player = NULL;
            }
            game->player2 = NULL;
            if (game->player1 != NULL) {
                send(game->player1->socket->fd, "DISCONNECT Player 2 disconnected.\n", 35, 0);
            }
        }
    }
}

void remove_game(struct game *games[], struct game *game) {
    for (int i = 0; i < MAX_GAMES; i += 1) {
        if (games[i] != NULL && games[i]->id == game->id) {
            free(game->board);
            free(game);
            games[i] = NULL;
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
