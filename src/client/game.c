#include <stdio.h>
#include <curses.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <ctype.h>
#include <sys/socket.h>

#include "game.h"
#include "macros.h"
#include "socket.h"
#include "../helpers.h"

#define ALT_BACKSPACE 127
#define OPTS_MENU_WIDTH 40
#define OPTS_MENU_HEIGHT 4

#define CREATE_GAME_OPT 0
#define JOIN_GAME_OPT 1



char *name, *shape;
struct game *game;
bool my_turn = false;

void init_game(char *ip, int port) {
    initscr();
    keypad(stdscr, true);
    cbreak();
    noecho();
    clear();

    int centerx = COLS / 2,
        centery = LINES/ 2;

    show_logo(centery - 11);
    print_str(centery, "Connecting to the server...");
    refresh();

    int socket = setup_server_socket(ip, port);
    pthread_t socket_thread;
    pthread_create(&socket_thread, NULL, socket_reader_thread, (void *) &socket);

    wait_for_message(); // Wait for welcome message

    clear_lines(centery, centery);

    set_initial_data(centery);

    print_str(centery + 1, "Choose an option wisely!");

    char *opts[2] = {"Create a new game", "Join a game"};
    int option = show_options_menu(centerx, centery + 3, opts, 2);

    clear_lines(centery + 1, centery + 1); // Clear the choose an option wisely title

    if (option == JOIN_GAME_OPT) {
        join_game(centery, socket);
    } else {
        create_game(centery, socket);
    }

    game->me = malloc(sizeof(struct player));
    game->me->name  = name;
    game->me->shape = shape;
    game->opponent = malloc(sizeof(struct player));

    // Initialize game
    while (1) {
        request_new_message();
        wait_for_message();
        if (strncmp(server_output, "GAME_START", 10) == 0) break;
        init_game_data();
    }

    game->board = malloc(sizeof(int) * game->size * game->size);
    for (int i = game->size * game->size - 1; i >= 0; i -= 1) game->board[i] = 0;

    // Game loop
    while (1) {
        if (!my_turn) {
            request_new_message();
            wait_for_message();
        }
        game_loop();
        render();
    }


    printf("Initialized game successfully");
    fflush(stdout);

    endwin();
}

void render() {
    clear();
    char str[200];

    int centery = LINES / 2;

    mvaddstr(0, 0, "You");
    sprintf(str, "Name: %s", game->me->name);
    mvaddstr(1, 0, str);
    sprintf(str, "Shape: %s", game->me->shape);
    mvaddstr(2, 0, str);
    sprintf(str, "Score: %d", game->my_wins);
    mvaddstr(3, 0, str);


    mvaddstr(5, 0, "Your Opponent");
    sprintf(str, "Name: %s", game->opponent->name);
    mvaddstr(6, 0, str);
    sprintf(str, "Shape: %s", game->opponent->shape);
    mvaddstr(7, 0, str);
    sprintf(str, "Score: %d", game->opponent_wins);
    mvaddstr(8, 0, str);

    // Print board
    int width = game->size * 2 + 1,
        left  = COLS / 2 - width / 2,
        top   = LINES / 2 - width / 2;

    WINDOW *board = newwin(
                              width,
                              width,
                              top,
                              left
                          );



    refresh();
    getch();
}

void game_loop() {
    if (strncmp(server_output, "BOARD", 5) == 0) {
        char *itemptr = get_server_command_value(server_output, server_output_size);
        itemptr -= 1; // include the space because on each iteration we'll skip a character to account for the commas
        for (int i = 0; i < game->size * game->size; i += 1) {
            game->board[i] = strtol(itemptr + 1, &itemptr, 10);
        }
    } else if (strncmp(server_output, "YOUR_TURN", 9) == 0) {
        my_turn = true;
    } else {
        fprintf(stderr, "Unrecongized server expression %s", server_output);
        exit(1);
    }
}

void init_game_data() {
    if (strncmp(server_output, "OPPONENT_INDICATOR", 18) == 0) {
        char *data = get_server_command_value(server_output, server_output_size);
        game->opponent_indicator = strtol(data, NULL, 10);
        free(data);
    } else if (strncmp(server_output, "YOUR_INDICATOR", 14) == 0) {
        char *data = get_server_command_value(server_output, server_output_size);
        game->my_indicator = strtol(data, NULL, 10);
        free(data);
    } else if (strncmp(server_output, "GAME_SIZE", 9) == 0) {
        char *data = get_server_command_value(server_output, server_output_size);
        game->size = strtol(data, NULL, 10);
        free(data);
    } else if (strncmp(server_output, "OPPONENT_SHAPE", 14) == 0) {
        game->opponent->shape = get_server_command_value(server_output, server_output_size);
    } else if (strncmp(server_output, "OPPONENT_NAME", 13) == 0) {
        game->opponent->name = get_server_command_value(server_output, server_output_size);
    } else if (strncmp(server_output, "SCORE", 5) == 0) {
        char *strptr = memchr(server_output, ' ', server_output_size);

        game->my_wins       = strtol(strptr, &strptr, 10);
        game->opponent_wins = strtol(strptr + 1, NULL, 10);
    } else {
        fprintf(stderr, "Unrecongized server expression %s", server_output);
        exit(1);
    }
}

char *get_server_command_value(char *str, int n) {
    char *data = memchr(str, ' ', n);
    if (data == NULL) return NULL;
    int l = n - (data - str);
    char *output = malloc(l + 1);
    memcpy(output, data, l);
    output[l] = '\0';
    return output;
}

void join_game(int centery, int socket) {
    int attemptsc = 0;
    while (1) {
        print_str(centery + 1, "Please enter the game's 4 digit code");
        char *codestr = read_str(centery + 3, 4, 4, isdigit);
        int code = strtol(codestr, NULL, 10);
        free(codestr);
        char msg[50];
        sprintf(msg, "join %d %s %s\n", code, name, shape);
        send(socket, msg, strlen(msg), 0);
        request_new_message();
        wait_for_message();
        clear_lines(centery + 2, centery + 3);
        if (strncmp(server_output, "NOT_FOUND_ERR", 13) == 0) {
            char str[100];
            sprintf(str, "The game you're trying to join does not exist. Attempt %d", ++attemptsc);
            print_str(centery + 2, str);
        } else if (strncmp(server_output, "SAME_SHAPE_ERR", 13) == 0) {
            char str[100];
            char *old_shape = shape;
            shape = NULL; // This is done to prevent the the value in old_shape from being freed in the do while loop
            clear_lines(centery + 3, centery + 3);
            sprintf(str, "You have the same shape as the other player \"%s\" Please pick a different one", old_shape);
            print_str(centery + 1, str);
            int size;
            do {
                free(shape);
                shape = read_str(centery + 3, 1, 2, NULL);
                clear_lines(centery + 3, centery + 3);
            } while (strcmp(shape, old_shape) == 0);
            free(old_shape);
            // Try joining the game again
            sprintf(msg, "join %d %s %s\n", code, name, shape);
            send(socket, msg, strlen(msg), 0);
            request_new_message();
            wait_for_message();
            game = malloc(sizeof(struct game));
            game->id = code;
            break;
        } else if (strncmp(server_output, "SUCCESS", 7) == 0) {
            game = malloc(sizeof(struct game));
            game->id = code;
            break;
        } else {
            fprintf(stderr, "Unrecongized server expression %s", server_output);
            exit(1);
        }
    }
}

void create_game(int centery, int socket) {
    print_str(centery + 1, "Please enter the size of the board (3-10)");
    int size;
    do {
        char *str = read_str(centery + 3, 1, 2, isdigit);
        size = strtol(str, NULL, 10);
        free(str);
        clear_lines(centery + 3, centery + 3);
    } while (size > 10 || size < 3);
    char msg[100];
    sprintf(msg, "create %d %s %s\n", size, name, shape);
    send(socket, msg, strlen(msg), 0);
    request_new_message();
    wait_for_message();
    clear_lines(centery + 1, centery + 3);
    if (strncmp(server_output, "Error", 5) == 0) {
        print_str(centery + 1, server_output);
        print_str(centery + 2, "Press any key to exit");
        getch();
        endwin();
        exit(0);
    } else {
        print_str(centery + 1, "Game created successfully. ");
        print_str(centery + 2, server_output);
        print_str(centery + 3, "Waiting for player2 to join...");
        game = malloc(sizeof(struct game));
        int code = strtol(memchr(server_output, ' ', strlen(server_output)), NULL, 10);
        game->id = code;
    }
}

void wait_for_message() {
    pthread_mutex_lock(&socket_lock);
    pthread_cond_wait(&socket_cond, &socket_lock);
}

void request_new_message() {
    pthread_mutex_unlock(&socket_lock);
}

void update_game_state(char *message) {

}

int show_options_menu(int posx, int posy, char **opts, int optsc) {
    int key, highlight = 0,
        marginx = posx - OPTS_MENU_WIDTH / 2,
        marginy = posy;


    WINDOW *menu = newwin(
                             OPTS_MENU_HEIGHT,
                             OPTS_MENU_WIDTH,
                             marginy,
                             marginx
                         );

    keypad(menu, true);

    box(menu, 0, 0);
    wrefresh(menu);

    while (key != '\n') {
        for (int i = 0; i < optsc; i += 1) {
            if (i == abs(highlight % optsc))
                wattron(menu, A_REVERSE);
            mvwprintw(menu, i + 1, OPTS_MENU_WIDTH / 2 - strlen(opts[i]) / 2, opts[i]);
            wattroff(menu, A_REVERSE);
        }
        key = wgetch(menu);

        switch (key) {
        case KEY_UP:
            highlight -= 1;
            break;
        case KEY_DOWN:
            highlight += 1;
            break;
        }
    }
    delwin(menu);
    clear_lines(posy, posy + optsc + 2); // +2 is for the borders

    return abs(highlight % optsc);
}

void set_initial_data(int posy) {
    print_str(posy + 1, "Welcome to Tic-Tac-Toe");
    sleep(1);
    print_str(posy + 2, "Brought to you by bender's ass");
    sleep(1);
    print_str(posy + 4, "Please Enter Your Name (3-50 characters)");

    name = read_str(posy + 5, 3, 50, NULL);

    clear_lines(posy + 4, posy + 6);

    print_str(posy + 4, "Please Enter Your Shape (1-3 characters)");
    shape = read_str(posy + 5, 1, 3, NULL);

    clear_lines(posy + 1, posy + 5);
}

void print_str(int posy, char *str) {
    mvaddstr(posy, COLS / 2 - strlen(str) / 2, str);
    refresh();
}

void clear_lines(int start, int end) {
    while (start <= end) {
        move(start++, 0);
        clrtoeol();
    }
    refresh();
}

char *read_str(int posy, int min, int max, int (*filterer)(int c)) {
    char *name = malloc(max + 1);
    int ch, chari = 0, centerx = COLS / 2;

    move(posy, centerx);
    refresh();

    while (1) {
        ch = getch();
        if (ch == '\n' && chari >= min) break;
        if (ch == ALT_BACKSPACE) {
            if (chari > 0) {
                chari -= 1;
                move(posy, 0);
                clrtoeol();
                mvaddnstr(posy, centerx - chari / 2, name, chari);
                refresh();
            }
        } else if (chari <= max - 1 && (filterer == NULL || filterer(ch))) {
            name[chari] = ch;
            chari += 1;

            mvaddnstr(posy, centerx - chari / 2, name, chari);
        }
    }
    name[chari] = '\0';

    return name;
}


void show_logo(int posy) {
    int row = MAX(posy, 1),
        col = MAX(COLS / 2 - LOGO_WIDTH / 2, 1);

    mvaddstr(row, col, LOGO_1);
    mvaddstr(row + 1, col, LOGO_2);
    mvaddstr(row + 2, col, LOGO_3);
    mvaddstr(row + 3, col, LOGO_4);
    mvaddstr(row + 4, col, LOGO_5);
    mvaddstr(row + 5, col, LOGO_6);
    mvaddstr(row + 6, col, LOGO_7);
    mvaddstr(row + 7, col, LOGO_8);
    mvaddstr(row + 8, col, LOGO_9);
}
