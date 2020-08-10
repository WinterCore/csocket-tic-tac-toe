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
        create_game(centery, socket);
    } else {
        join_game(centery, socket);
    }

    getch();

    endwin();
}

void create_game(int centery, int socket) {
    int attemptsc = 0;
    while (1) {
        print_str(centery + 1, "Please enter the game's 4 digit code");
        clear_lines(centery + 3, centery + 3);
        int code = strtol(read_str(centery + 3, 4, 4, isdigit), NULL, 10);
        char msg[50];
        sprintf(msg, "join %d %s %s\n", code, name, shape);
        send(socket, msg, strlen(msg), 0);
        request_new_message();
        wait_for_message();
        if (strncmp(server_output, "SUCCESS", 7) == 0) break;
        char str[100];
        sprintf(str, "The game you're trying to join does not exist. Attemp %d", ++attemptsc);
        print_str(centery + 2, str);
    }

    printf("%s\n", server_output);
    fflush(stdout);
    sleep(5);
}

void join_game(int centery, int socket) {
    print_str(centery + 1, "Please enter the size of the board (3-10)");
    int size;
    do {
        size = strtol(read_str(centery + 3, 1, 2, isdigit), NULL, 10);
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
