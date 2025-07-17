#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>
#include <stdbool.h>

extern bool debug;

#define DEBUG_PRINT(fmt, ...) \
    do { if (debug) fprintf(stderr, fmt, ##__VA_ARGS__); } while (0)

#endif // DEBUG_H