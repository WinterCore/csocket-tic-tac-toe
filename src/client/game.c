#include <stdio.h>
#include <curses.h>
#include <string.h>
#include <unistd.h>

#include "game.h"
#include "macros.h"
#include "../helpers.h"

#define ALT_BACKSPACE 127

void init_game(int sock) {
    initscr();
    keypad(stdscr, true);
    cbreak();
    noecho();
    clear();

    int centery = LINES / 2, centerx = COLS / 2;

    show_logo();
    refresh();
    sleep(1);
    char message[50];
    sprintf(message, "%s", "Welcome to Tic-Tac-Toe");
    mvaddstr(centery + 1, centerx - strlen(message) / 2, message);
    refresh();
    sleep(1);
    sprintf(message, "%s", "Brought to you by bender's ass");
    mvaddstr(centery + 2, centerx - strlen(message) / 2, message);
    refresh();

    sleep(1);

    sprintf(message, "%s", "Please Enter Your Name: ");
    mvaddstr(centery + 4, centerx - strlen(message) / 2, message);
    refresh();

    move(centery + 6, centerx);

    char name[40]; // chars
    int ch, chari = 0;
    while ((ch = getch()) != '\n') {
        if (chari >= 40) {
            continue;
        }
        if (ch == ALT_BACKSPACE) {
            if (chari > 0) {
                chari -= 1;
                move(centery + 6, 0);
                refresh();
                clrtoeol();
                mvaddnstr(centery + 6, centerx - chari / 2, name, chari);
                refresh();
            }
        } else {
            name[chari] = ch;
            chari += 1;

            mvaddnstr(centery + 6, centerx - chari / 2, name, chari);
        }
    }
    name[chari] = '\0';


    refresh();
    getch();
    endwin();
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
