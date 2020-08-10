#ifndef GAME_H
#define GAME_H

#include <pthread.h>
#include <stdbool.h>

extern int server_out_size;
extern char server_output[200];

extern pthread_cond_t socket_cond;
extern pthread_mutex_t socket_lock;



void init_game(char *ip, int port);
void set_initial_data(int posy);

void show_logo(int posy);
void print_str(int posy, char *str);
char *read_str(int posy, int min, int max, int (*filterer)(int c));
void clear_lines(int start, int end);
int show_options_menu(int posx, int posy, char **opts, int optsc);
void update_game_state(char *message);
void wait_for_message();
void request_new_message();
void create_game(int centery, int socket);
void join_game(int centery, int socket);
bool game_loop();

#endif
