#ifndef STRUCTS_H
#define STRUCTS_H

#include <stdint.h>

struct Request { 
    uint32_t    addr;
    uint32_t    data;
    uint8_t        w;
};

struct Result {
    uint32_t    cycles;
    uint32_t    misses;
    uint32_t      hits;
};

//CHANGE AS NEEDED
enum DefaultSimulationValues {
    CYCLES           = 1000,
    NUM_CACHE_LEVELS = 1   ,
    CACHE_LINE_SIZE  = 4   ,
    NUM_LINES_L1     = 8   ,
    NUM_LINES_L2     = 8   ,
    NUM_LINES_L3     = 8   ,
    LATENCY_CACHE_L1 = 10  ,
    LATENCY_CACHE_L2 = 20  ,
    LATENCY_CACHE_L3 = 30  ,
    MAPPING_STRATEGY = 0   ,
};

#endif