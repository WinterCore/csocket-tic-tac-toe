#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "socket.h"

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
