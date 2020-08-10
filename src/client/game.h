#ifndef GAME_H
#define GAME_H

#include <pthread.h>
#include <stdbool.h>

extern int server_output_size;
extern char server_output[200];

extern pthread_cond_t socket_cond;
extern pthread_mutex_t socket_lock;


struct game {
    int id;
    int my_wins;
    int opponent_wins;
    int opponent_indicator;
    int my_indicator;
    struct player *me;
    struct player *opponent;
    int size;
    int *board;
};

struct player {
    char *name;
    char *shape;
};


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
void init_game_data();
void game_loop();
void render();
char *get_server_command_value(char *str, int n);

#endif
