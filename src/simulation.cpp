#include "../include/simulation.hpp"
#include <iostream>
#include "../util/helper_functions.h"
#include "../include/cache.hpp"
#include "../include/structs/test.h"
#include "../include/structs/debug.h"


void print_simulation_results(Result result, uint32_t cycles, const char* tracefile,
                              uint8_t numCacheLevels, uint32_t cachelineSize,
                              uint32_t numLinesL1, uint32_t numLinesL2,
                              uint32_t numLinesL3, uint32_t latencyCacheL1,
                              uint32_t latencyCacheL2, uint32_t latencyCacheL3,
                              uint8_t mappingStrategy) {
    printf("\n\t\t======SIMULATION PARAMETRS======\n\
            \tCycles: %u\n\
            \tTracefile: %s\n\
            \tNumber of cache levels: %u\n\
            \tCacheline size: %u\n\
            \tNumber of L1 lines: %u\n\
            \tNumber of L2 lines: %u\n\
            \tNumber of L3 lines: %u\n\
            \tLatency of L1 cache: %u\n\
            \tLatency of L2 cache: %u\n\
            \tLatency of L3 cache: %u\n\
            \tMapping strategy: %s\n",
            cycles,
            tracefile ? tracefile : "none",
            numCacheLevels,
            cachelineSize,
            numLinesL1,
            numLinesL2,
            numLinesL3,
            latencyCacheL1,
            latencyCacheL2,
            latencyCacheL3,
            mappingStrategy == 1 ? "Fully associative" : "Direct mapped"); 

    printf("\n\t\t======SIMULATION RESULTS======\n\
            \tCycles: %u\n\
            \tHits: %u\n\
            \tMisses: %u\n\n",
            result.cycles, result.hits, result.misses);
}

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
    sc_signal<bool> r, w, stop;

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

    MAIN_MEMORY main_memory("main_memory", cachelineSize);

    main_memory.clk(clk);


    sc_signal<uint32_t> mem_addr_sig, mem_wdata_sig, mem_rdata_sig;
    std::vector<sc_signal<uint8_t>> mem_cacheline_sig(cachelineSize);
    sc_signal<bool> mem_r_sig, mem_w_sig, mem_ready_sig;

    main_memory.addr(mem_addr_sig);
    cache.mem_addr(mem_addr_sig);
    main_memory.wdata(mem_wdata_sig);
    cache.mem_wdata(mem_wdata_sig);
    main_memory.r(mem_r_sig);
    cache.mem_r(mem_r_sig);
    main_memory.w(mem_w_sig);
    cache.mem_w(mem_w_sig);
    main_memory.rdata(mem_rdata_sig);

    for (uint32_t i = 0; i < cachelineSize; i++)
    {
      main_memory.cacheline[i](mem_cacheline_sig[i]);
      cache.mem_cacheline[i](mem_cacheline_sig[i]);
    }

    cache.mem_ready(mem_ready_sig);
    main_memory.ready(mem_ready_sig);
    main_memory.stop(stop);
    cache.mem_stop(stop);

    Result result;
    result.cycles = 0;
    result.hits = 0;
    result.misses = 0;
    Request request;

    sc_trace_file *trace = NULL;

    if (tracefile != NULL && is_valid_filename(tracefile))
    {
        trace = sc_create_vcd_trace_file(tracefile);
        sc_trace(trace, clk, "clk");
        sc_trace(trace, addr, "addr");
        sc_trace(trace, wdata, "wdata");
        sc_trace(trace, r, "r");
        sc_trace(trace, w, "w");
        sc_trace(trace, rdata, "rdata");
        sc_trace(trace, ready, "ready");
        sc_trace(trace, miss, "miss");
    }

    for (size_t request_index = 0; request_index < numRequests; request_index++) {
        request = requests[request_index];

        DEBUG_PRINT("SIMULATION: Request %zu: type=%s, addr = 0x%08X, data=0x%08X\n", request_index + 1,
            request.w ? "W" : "R",
            request.addr,
            request.data);

        addr.write(request.addr);
        wdata.write(request.data);
        r.write(!request.w);
        w.write(request.w);

        do {
            DEBUG_PRINT("Clock %u: \n", result.cycles);
            
            if (result.cycles >= cycles) {
                printf("Limit of cycles reached, stopping simulation.\n");
                return result;
            }
            result.cycles++;

            sc_start(10, SC_NS);

        } while (!ready.read());

        // DEBUG_PRINT("SIMULATION: Read data: %u\n", cache.rdata.read());
        if (test) {
            // if (debug) cache.print_caches();
            if (!request.w && request.data != cache.rdata.read()) {
                print_simulation_results(result, cycles, tracefile,
                                    numCacheLevels, cachelineSize,
                                    numLinesL1, numLinesL2,
                                    numLinesL3, latencyCacheL1,
                                    latencyCacheL2, latencyCacheL3,
                                    mappingStrategy);
                std::cerr << "\t\tError: Read data does not match expected data!\n";
                printf("\t\tExpected data: %u, Read data: %u on Request %zu: type=%s, addr = 0x%08X, data=0x%08X\n", request.data, cache.rdata.read(), request_index + 1,
                        request.w ? "W" : "R",
                        request.addr,
                        request.data);
                return result;
            }
        }
        if (miss.read()) result.misses++;
        else result.hits++;
    }

    if (tracefile != NULL && trace != NULL)
        sc_close_vcd_trace_file(trace);
    
    cache.print_caches();

    print_simulation_results(result, cycles, tracefile,
                              numCacheLevels, cachelineSize,
                              numLinesL1, numLinesL2,
                              numLinesL3, latencyCacheL1,
                              latencyCacheL2, latencyCacheL3,
                              mappingStrategy);

    printf("\t\tSIMULATION: Simulation finished successfully with %.2f hit rate and %.2f%% efficiency\n", static_cast<double>(result.hits)/static_cast<double>(numRequests), 100.0 * static_cast<double>(numRequests * 100) / static_cast<double>(result.cycles) - 100.0);

    return result;
}
