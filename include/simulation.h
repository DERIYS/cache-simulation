#ifndef SIMULATION_HPP
#define SIMULATION_HPP

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

struct Result run_simulation (
    uint32_t             cycles,
    const char*       tracefile,
    uint8_t      numCacheLevels,
    uint32_t      cachelineSize,
    uint32_t         numLinesL1,
    uint32_t         numLinesL2,
    uint32_t         numLinesL3,
    uint32_t     latencyCacheL1,
    uint32_t     latencyCacheL2,
    uint32_t     latencyCacheL3,
    uint8_t     mappingStrategy,
    uint32_t        numRequests,
    struct Request*    requests
);

#endif // SIMULATION_HPP