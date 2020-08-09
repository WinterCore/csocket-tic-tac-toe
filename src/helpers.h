#ifndef HELPERS_H
#define HELPERS_H
#include <stdbool.h>

#define MIN(x, y) x > y ? y : x
#define MAX(x, y) x > y ? x : y

void strtolower(char *str);
int read_word(char *str, int *start);
int random_number();
void sstrncpy(char *dest, char *src, int n, int skip);
char *slicestr(char *str, int start, int end);
bool str_is_numeric(char *str);

#endif
