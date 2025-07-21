#ifndef SIMULATION_HPP
#define SIMULATION_HPP

#include "structs/request.h"
#include "structs/result.h"

#ifdef __cplusplus
extern "C" {
#endif

Result run_simulation (
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
    Request*    requests
);

void print_simulation_results(Result result, uint32_t cycles, const char* tracefile,
                              uint8_t numCacheLevels, uint32_t cachelineSize,
                              uint32_t numLinesL1, uint32_t numLinesL2,
                              uint32_t numLinesL3, uint32_t latencyCacheL1,
                              uint32_t latencyCacheL2, uint32_t latencyCacheL3,
                              uint8_t mappingStrategy);

#ifdef __cplusplus
}
#endif

#endif // SIMULATION_HPP