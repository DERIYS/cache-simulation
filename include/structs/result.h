#ifndef RESULT_H
#define RESULT_H

#include <stdint.h>

typedef struct {
    uint32_t    cycles;
    uint32_t    misses;
    uint32_t      hits;
 } Result;

#endif // RESULT_H