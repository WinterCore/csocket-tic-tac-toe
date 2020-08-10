#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include "socket.h"

#define READ_BUFFER_SIZE 100

int server_output_size = 0;
char server_output[200];

pthread_cond_t socket_cond  = PTHREAD_COND_INITIALIZER;
pthread_mutex_t socket_lock = PTHREAD_MUTEX_INITIALIZER;


int setup_server_socket(char *str, int port) {
    int sock = 0;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        fprintf(stderr, "Failed to create client server");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port   = htons(port);

    if (inet_pton(AF_INET, str, &serv_addr.sin_addr) <= 0) {
        fprintf(stderr, "Invalid address: Address not supported\n");
        exit(EXIT_FAILURE);
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        fprintf(stderr, "Connection Failed\n");
        exit(EXIT_FAILURE);
    }

    return sock;
}


void *socket_reader_thread(void *args) {

    int socket = *(int *) args;
                printf("Here");
                fflush(stdout);
    char read_buffer[READ_BUFFER_SIZE];
    int read_buffer_size = 0;

    while (1) {
        int buf_remain = READ_BUFFER_SIZE - read_buffer_size;
        if (buf_remain == 0) {
            // TODO
        }

        int nread = read(socket, &read_buffer[read_buffer_size], buf_remain);
        if (nread < 0) {
            if (errno == EAGAIN) {
                break;
            } else {
                // TODO: handle error
            }
        } else if (nread == 0) {
            // TODO: Server disconnected us (I think)
        } else {
            read_buffer_size += nread;
            char *line_start = read_buffer;
            char *line_end;
            while ((line_end = memchr(line_start, '\n', read_buffer_size - (line_start - read_buffer)))) {
                if (strncmp(line_start, "ACK", 3) == 0) {
                    line_start = line_end + 1;
                    continue;
                }
                pthread_mutex_lock(&socket_lock);
                *line_end = '\0';
                memcpy(server_output, line_start, (line_end - line_start));
                server_output[line_end - line_start] = '\0';
                line_start = line_end + 1;
                pthread_cond_signal(&socket_cond);
                pthread_mutex_unlock(&socket_lock);
                struct timespec time = {.tv_sec = 0, .tv_nsec = 100000000};
                nanosleep(&time, NULL);
                // TODO: Find something other than nanosleep to allow the other process to continue
            }
            read_buffer_size -= line_start - read_buffer;
            memmove(read_buffer, line_start, read_buffer_size);
        }
    }

    return NULL;
}
