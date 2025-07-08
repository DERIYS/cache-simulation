#include"../include/simulation.hpp"
#include<iostream>

/* 
   * @brief                     C++ function to start a simulation with SystemC modules
   *
   * @param cycles              Amount of cycles in which the simulation should perform
   * @param tracefile           Name of a trace file to be created. If passed as NULL, then no trace file should be created 
   * @param numCacheLevels      Number of active cache levels needed for simulation
   * @param cachelineSize       Size of a single cache line
   * @param numLinesL1          Number of lines of the L1 cache
   * @param numLinesL2          Number of lines of the L2 cache
   * @param numLinesL3          Number of lines of the L3 cache
   * @param latencyCacheL1      Latency of L1 cache
   * @param latencyCacheL2      Latency of L2 cache
   * @param latencyCacheL3      Latency of L3 cache
   * @param mappingStrategy     Chosen mapping strategy for the simulation (0=Direct-mapped, 1=Fully associative)
   * @param numRequests         Number of requests to process
   * @param requests            Pointer to requests
   * 
   * @return         
   *         
*/
Result run_simulation (
    uint32_t             cycles,
    const char*       tracefile, /*if tracefile is NULL, don't create it*/
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
) 
{
    /*TODO*/
    std::cout << "Hello from SystemC" << std::endl;
    return Result{};
}
