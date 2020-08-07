#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "helpers.h"


void strtolower(char *str) {
    for (int i = 0; str[i]; i += 1)
        str[i] = tolower(str[i]);
}

// Take the start position of the word and return the end position
int read_word(char *str, int *start) {
    bool is_in_quotes = false;
    int i = *start;
    while (str[i] == ' ') i += 1;
    *start = i;
    for (; str[i] && ((str[i] != ' ' || is_in_quotes) && str[i] != '\n' && str[i] != '\t'); i += 1) {
        if (str[i] == '"')
            is_in_quotes = !is_in_quotes;
    }
    return i;
}

int random_number() {
    time_t t;
    srand((unsigned) time(&t));
    return 1000 + (rand() % 9000);
}

void sstrncpy(char *dest, char *src, int n, int skip) {
    if (n == 0) return;
    while (skip-- > 0)
        src++;
    strncpy(dest, src, n);
}

char *slicestr(char *str, int start, int end) {
    int l = end - start;
    char *output = malloc(l + 1);
    sstrncpy(output, str, l, start);
    output[l] = '\0';
    return output;
}

bool str_is_numeric(char *str) {
    while (*str)
        if (!isdigit(*str++))
            return false;
    return true;
}
