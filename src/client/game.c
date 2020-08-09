#include <stdio.h>
#include <curses.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

#include "game.h"
#include "macros.h"
#include "../helpers.h"

#define ALT_BACKSPACE 127
#define OPTS_MENU_WIDTH 40
#define OPTS_MENU_HEIGHT 4

#define CREATE_GAME_OPT 0
#define JOIN_GAME_OPT 1

char *name = "Hasan", *shape = "x";

void init_game(int sock) {
    initscr();
    keypad(stdscr, true);
    cbreak();
    noecho();
    clear();

    int centerx = COLS / 2,
        centery = LINES/ 2;

    // set_initial_data();

    print_str(centery - 3, "Choose an option wisely!");

    char *opts[2] = {"Create a new game", "Join a game"};
    int option = show_options_menu(centerx, centery + 1, opts, 2);

    clear();


    getch();
    endwin();
}

int show_options_menu(int posx, int posy, char **opts, int optsc) {
    int key, highlight = 0,
        marginx = posx - OPTS_MENU_WIDTH / 2,
        marginy = posy - OPTS_MENU_HEIGHT / 2;


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

    return abs(highlight % optsc);
}

void set_initial_data() {
    int centery = LINES / 2;

    show_logo();
    refresh();
    sleep(1);
    print_str(centery + 1, "Welcome to Tic-Tac-Toe");
    sleep(1);
    print_str(centery + 2, "Brought to you by bender's ass");
    sleep(1);
    print_str(centery + 4, "Please Enter Your Name");

    name = read_str(centery + 5, 40);

    clear_lines(centery + 4, centery + 6);

    print_str(centery + 4, "Please Enter Your Shape");
    shape = read_str(centery + 5, 3);
}

void print_str(int ypos, char *str) {
    mvaddstr(ypos, COLS / 2 - strlen(str) / 2, str);
    refresh();
}

void clear_lines(int start, int end) {
    while (start <= end) {
        move(start++, 0);
        clrtoeol();
    }
    refresh();
}

char *read_str(int ypos, int max) {
    char *name = malloc(max + 1);
    int ch, chari = 0, centerx = COLS / 2;

    move(ypos, centerx);
    refresh();

    while ((ch = getch()) != '\n') {
        if (chari >= max + 1) continue;

        if (ch == ALT_BACKSPACE) {
            if (chari > 0) {
                chari -= 1;
                move(ypos, 0);
                clrtoeol();
                mvaddnstr(ypos, centerx - chari / 2, name, chari);
                refresh();
            }
        } else {
            name[chari] = ch;
            chari += 1;

            mvaddnstr(ypos, centerx - chari / 2, name, chari);
        }
    }
    name[chari] = '\0';

    return name;
}


void show_logo() {
    int row = MAX(LINES / 2 - 10, 1),
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
