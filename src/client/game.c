#include <stdio.h>
#include <curses.h>
#include <string.h>

#include "game.h"

void init_game(int sock) {
    initscr();
    cbreak();
    noecho();
    clear();

    char *welcome = "Welcome to tic tac toe";
    mvaddstr(LINES / 2 - 5, COLS / 2 - strlen(welcome) / 2, welcome);
    move(LINES - 1, COLS - 1);
    refresh();
    getch();
    endwin();
}
