#ifndef GAME_H
#define GAME_H

void init_game(char *ip, int port);
void set_initial_data(int posy);

void show_logo(int posy);
void print_str(int posy, char *str);
char *read_str(int posy, int min, int max, int (*filterer)(int c));
void clear_lines(int start, int end);
int show_options_menu(int posx, int posy, char **opts, int optsc);

#endif
