#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>
#include <stdbool.h>

#ifdef ENABLE_DEBUG

extern bool debug;

#define DEBUG_PRINT(fmt, ...) \
    do { if (debug) fprintf(stderr, fmt, ##__VA_ARGS__); } while (0)

#else

#define DEBUG_PRINT(...) ((void)0)

#endif // ENABLE_DEBUG

#endif // DEBUG_H