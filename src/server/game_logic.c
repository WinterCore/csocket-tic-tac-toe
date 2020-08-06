#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "game.h"
#include "game_logic.h"

char *board_to_str(int board[], int size) {
    int l = size * size * 2;
    char *str = malloc(l);
    for (int i = 0, j = 0; i < size * size; i += 1, j += 2) {
        str[j] = board[i] + '0';
        str[j + 1] = ',';
    }
    str[l - 1] = '\0';
    return str;
}

bool place_move(int board[], int size, int position, PLAYER_NO player) {
    if (position < size * size && board[position] == PLAYER_EMPTY)
        board[position] = player;
    return false;
}

bool check_horizontal(int board[], int size) {
    for (int i = 0; i < size; i += 1) {
        if (board[i * size] != PLAYER_EMPTY) {
            bool check = true;
            for (int j = 0; j < size; j += 1) {
                if (board[i * size + j] != board[i * size]) {
                    check = false;
                    break;
                }
            }
            if (check) return true;
        }
    }
    return false;
}

bool check_vertical(int board[], int size) {
    for (int i = 0; i < size; i += 1) {
        if (board[i] != PLAYER_EMPTY) {
            bool check = true;
            for (int j = 0; j < size; j += 1) {
                if (board[j * size + i] != board[i]) {
                    check = false;
                    break;
                }
            }
            if (check) return true;
        }
    }
    return false;
}

bool check_diagonal(int board[], int size) {
    if (board[0] == PLAYER_EMPTY || board[size - 1] == PLAYER_EMPTY) return false;

    bool check = true;
    // top left to bottom right
    for (int i = 0; i < size; i += 1) {
        if (board[i * size + i] != board[0]) {
            check = false;
            break;
        }
    }
    if (check) return true;
    // top right to bottom left
    for (int i = 0; i < size; i += 1) {
        if (board[i * size + (size - i - 1)] == board[0]) {
            check = false;
            break;
        }
    }
    return check;
}

bool is_board_full(int board[], int size) {
    for (int i = 0; i < size * size; i += 1) {
        if (board[i] == PLAYER_EMPTY) {
            return false;
        }
    }
    return true;
}
