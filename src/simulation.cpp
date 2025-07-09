#include "../include/simulation.hpp"
#include "../include/cache.hpp"
#include <iostream>

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
Result run_simulation(
    uint32_t cycles,
    const char *tracefile, /*if tracefile is NULL, don't create it*/
    uint8_t numCacheLevels,
    uint32_t cachelineSize,
    uint32_t numLinesL1,
    uint32_t numLinesL2,
    uint32_t numLinesL3,
    uint32_t latencyCacheL1,
    uint32_t latencyCacheL2,
    uint32_t latencyCacheL3,
    uint8_t mappingStrategy,
    uint32_t numRequests,
    Request *requests)
{
    CACHE cache("cache",
                numCacheLevels,
                cachelineSize,
                numLinesL1,
                numLinesL2,
                numLinesL3,
                latencyCacheL1,
                latencyCacheL2,
                latencyCacheL3,
                mappingStrategy);

    sc_clock clk("clk", 10, SC_NS);

    sc_signal<uint32_t> addr, wdata;
    sc_signal<bool> r, w;

    sc_signal<uint32_t> rdata;
    sc_signal<bool> ready, miss;

    cache.clk(clk);
    cache.addr(addr);
    cache.wdata(wdata);
    cache.r(r);
    cache.w(w);

    cache.rdata(rdata);
    cache.ready(ready);
    cache.miss(miss);

    Result result;
    result.cycles = 0;
    result.hits = 0;
    result.misses = 0;

    Request request;

    for (size_t request_index = 0; request_index < numRequests; request_index++)
    {
        request = requests[request_index];
        addr.write(request.addr);
        wdata.write(request.data);
        r.write(!request.w);
        w.write(request.w);
        printf("Request no. %i: ADDR: %u, WDATA: %u, R: %u, W: %u\n", request_index, request.addr, request.data, !request.w, request.w);
        do 
        {
            std::cout << "going in while on cycle: " << result.cycles << std::endl;
            if (result.cycles >= cycles) {
                std::cout<<"leaving bc cycles"<<std::endl;
                return result;
            }
            result.cycles++;
            sc_start(10, SC_NS);
        } while (!ready.read());
        
        printf("Read data: %u\n", cache.rdata.read());
        cache.print_caches();
        if (miss.read())
            result.misses++;
        else
            result.hits++;
    }

    return result;
}
