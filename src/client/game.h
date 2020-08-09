#ifndef GAME_H
#define GAME_H

void init_game(int sock);
void set_initial_data();

void show_logo();
void print_str(int ypos, char *str);
char *read_str(int ypos, int max);
void clear_lines(int start, int end);
int show_options_menu(int pos, int posy, char **opts, int optsc);

#endif
