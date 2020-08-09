#ifndef SOCKET_H
#define SOCKET_H

int setup_server_socket(char *str, int port);
void *socket_reader_thread(void *args);

#endif
