#ifndef SIMULATION_HPP
#define SIMULATION_HPP

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

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

#ifdef __cplusplus
}
#endif

#endif // SIMULATION_HPP