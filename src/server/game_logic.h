#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H
#include <stdbool.h>

#include "game.h"

char *board_to_str(int board[], int size);
bool place_move(int board[], int size, int position, PLAYER_NO player);

bool check_horizontal(int board[], int size);
bool check_vertical(int board[], int size);
bool check_diagonal(int board[], int size);
bool is_board_full(int board[], int size);

#endif
