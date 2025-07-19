#ifndef CACHE_HPP
#define CACHE_HPP

#include "cache_layer.hpp"
#include "multiplexer.hpp"
#include "main_memory.hpp"
#include "structs/debug.h"
#include <systemc>
#include <map>
#include <vector>
#include <memory>

// Define the zero time for cache operations, needed for waiting in cache operations
#define CACHE_ZERO_TIME 1, SC_NS

using namespace sc_core;

SC_MODULE(CACHE)
{
  // inputs
  sc_in<bool> clk;

  sc_in<uint32_t> addr, wdata;
  sc_in<bool> r, w;

  sc_in<bool> mem_ready;
  std::vector<sc_in<uint8_t>> mem_cacheline;

  // outputs
  sc_out<uint32_t> rdata;
  sc_out<bool> ready, miss;

  sc_out<uint32_t> mem_addr, mem_wdata;
  sc_out<bool> mem_r, mem_w;

  // modules
  std::vector<std::unique_ptr<CACHE_LAYER>> L;
  MAIN_MEMORY main_memory;
  MULTIPLEXER_BOOLEAN cache_miss_mux, cache_ready, r_mux, w_mux;
  MULTIPLEXER_I32 cache_data, addr_mux, wdata_mux;

  // parameters
  uint8_t num_cache_levels;
  uint32_t cacheline_size, num_lines_L1, num_lines_L2, num_lines_L3;
  uint32_t latency_cache_L1, latency_cache_L2, latency_cache_L3;
  uint8_t mapping_strategy;

  // signals
  sc_signal<uint32_t> addr_mux_in, addr_mux_out[3], wdata_mux_in, wdata_mux_out[3];
  sc_signal<bool> r_mux_in, r_mux_out[3], w_mux_in, w_mux_out[3];

  sc_signal<uint32_t> mem_addr_sig, mem_wdata_sig;
  std::vector<sc_signal<uint8_t>> mem_cacheline_sig;
  sc_signal<bool> mem_r_sig, mem_w_sig, mem_ready_sig;
  // multiplexer signals
  sc_signal<uint8_t> data_mux_select, miss_mux_select, ready_mux_select, select_zero;
  sc_signal<uint32_t> cache_data_in[3], cache_data_out;
  sc_signal<bool> cache_miss_in[3], cache_miss_out, cache_ready_in[3], cache_ready_out;

  SC_CTOR(CACHE);

  CACHE(sc_module_name name, uint8_t num_cache_levels, uint32_t cacheline_size, uint32_t num_lines_L1, uint32_t num_lines_L2,
        uint32_t num_lines_L3, uint32_t latency_cache_L1, uint32_t latency_cache_L2, uint32_t latency_cache_L3, uint8_t mapping_strategy)
      : sc_module(name),
        L(num_cache_levels),
        num_cache_levels(num_cache_levels),
        cacheline_size(cacheline_size),
        num_lines_L1(num_lines_L1), num_lines_L2(num_lines_L2), num_lines_L3(num_lines_L3),
        latency_cache_L1(latency_cache_L1), latency_cache_L2(latency_cache_L2), latency_cache_L3(latency_cache_L3),
        mapping_strategy(mapping_strategy),
        cache_data("cacheData", num_cache_levels, 1),
        cache_miss_mux("cacheMiss", num_cache_levels, 1),
        cache_ready("cacheReady", num_cache_levels, 1),
        r_mux("rMil", 1, num_cache_levels),
        w_mux("wMul", 1, num_cache_levels),
        addr_mux("addrMul", 1, num_cache_levels),
        wdata_mux("wdataMul", 1, num_cache_levels),
        main_memory("main_memory", cacheline_size),
        mem_cacheline_sig(cacheline_size),
        mem_cacheline(cacheline_size)
  {
    switch (num_cache_levels) {
    case 3:
      L[2] = (std::make_unique<CACHE_LAYER>("L3", latency_cache_L3, num_lines_L3, cacheline_size, mapping_strategy, 3));
    case 2:
      L[1] = (std::make_unique<CACHE_LAYER>("L2", latency_cache_L2, num_lines_L2, cacheline_size, mapping_strategy, 2));
    case 1:
      L[0] = (std::make_unique<CACHE_LAYER>("L1", latency_cache_L1, num_lines_L1, cacheline_size, mapping_strategy, 1));
      break;
    default:
      throw std::runtime_error("Number of Cache Levels must be in range [1;3].\n");
      break;
    }
    // set up mux for input data. 1 input, numCacheLevels output. each output for each cache level. select-bit everywhere = 0
    // input data will be writen to mux in behavior()
    select_zero.write(0);
    addr_mux.in[0](addr_mux_in);
    addr_mux.select(select_zero);
    wdata_mux.in[0](wdata_mux_in);
    wdata_mux.select(select_zero);
    r_mux.in[0](r_mux_in);
    r_mux.select(select_zero);
    w_mux.in[0](w_mux_in);
    w_mux.select(select_zero);

    // connect ports of each cache level
    for (int i = 0; i < num_cache_levels; i++)
    {
      L[i]->clk(clk);
      // use outputs from mux as input data
      L[i]->addr(addr_mux_out[i]);
      addr_mux.out[i](addr_mux_out[i]);
      L[i]->wdata(wdata_mux_out[i]);
      wdata_mux.out[i](wdata_mux_out[i]);
      L[i]->r(r_mux_out[i]);
      r_mux.out[i](r_mux_out[i]);
      L[i]->w(w_mux_out[i]);
      w_mux.out[i](w_mux_out[i]);
      // conncecnt to input of mux. numCacheLevels inputs, 1 output
      L[i]->data(cache_data_in[i]);
      cache_data.in[i](cache_data_in[i]);
      L[i]->miss(cache_miss_in[i]);
      cache_miss_mux.in[i](cache_miss_in[i]);
      L[i]->ready(cache_ready_in[i]);
      cache_ready.in[i](cache_ready_in[i]);
    }

    // connect output from mux, to get data from one of the cache levels
    cache_data.out[0](cache_data_out);
    cache_data.select(data_mux_select);
    cache_miss_mux.out[0](cache_miss_out);
    cache_miss_mux.select(miss_mux_select);
    cache_ready.out[0](cache_ready_out);
    cache_ready.select(ready_mux_select);

    // connect inputs/outputs for main_memory module
    main_memory.clk(clk);
    main_memory.addr(mem_addr_sig);
    mem_addr(mem_addr_sig);
    main_memory.wdata(mem_wdata_sig);
    mem_wdata(mem_wdata_sig);
    main_memory.r(mem_r_sig);
    mem_r(mem_r_sig);
    main_memory.w(mem_w_sig);
    mem_w(mem_w_sig);
    for (int i = 0; i < cacheline_size; i++)
    {
      main_memory.cacheline[i](mem_cacheline_sig[i]);
      mem_cacheline[i](mem_cacheline_sig[i]);
    }
    main_memory.ready(mem_ready_sig);
    mem_ready(mem_ready_sig);

    SC_THREAD(behaviour);
    sensitive << clk.pos();
  }

  /* * @brief Prints the internal memory of each cache level.
   * This function iterates through each cache level and prints its internal memory contents.
   * Used for debugging and verifying the state of the cache hierarchy.
   *
   * @note This function is intended for debugging purposes and may not be suitable for production use.
  */
  void print_caches()
  {
    for (int i = 0; i < num_cache_levels; i++)
    {
      L[i]->print_internal_memory(i + 1);
    }
    std::cout << "\n";
  }

  void set_mux_signals()
  {
    // set input signals in multiplexers, which forward them to each cache level
    w_mux_in.write(w.read());
    r_mux_in.write(r.read());
    wdata_mux_in.write(wdata.read());
    addr_mux_in.write(addr.read());
    mem_addr.write(addr.read());
    mem_wdata.write(wdata.read());
    mem_r.write(r.read());
    mem_w.write(w.read());
  }

  void reset_signals() {
    ready.write(false);
    miss.write(false);
    rdata.write(0);
  }

  void behaviour()
  {
    while (true)
    {
      wait();
      DEBUG_PRINT("MAIN: Cache behaviour thread running...\n");
      reset_signals();
      set_mux_signals();
      wait(SC_ZERO_TIME);
      DEBUG_PRINT("MAIN: Input signals set in multiplexers.\n");

      if (r.read()) doRead();
      if (w.read()) doWrite();

      ready.write(true);
      miss.write(cache_miss_out.read());
      if (!miss.read()) rdata.write(cache_data_out.read());
    }
  }

  void wait_zero_cachetime() {
    wait(CACHE_ZERO_TIME);
  }

  /**
   * @brief Handles a read request in the cache hierarchy.
   *
   * This function initiates a read operation by signaling the main memory and propagating the request
   * through all cache levels. It waits for each cache level to become ready and checks for a cache hit.
   * If a hit occurs in any cache level, it updates the multiplexer select signals to route the hit data
   * to the output, sets stop/idle flags for other cache levels and main memory, and returns the data.
   * If all cache levels miss, it waits for the main memory to become ready, retrieves the cacheline from
   * memory, writes it to all cache levels, and outputs the requested word from the newly filled cacheline.
   */
  void doRead()
  {
    bool hit = false;

    mem_r.write(true); // setting read signal to true, so that MM will start reading data
    wait(); // wait for the next clock cycle so that memory and cache levels start processing the request
    mem_r.write(false); // setting read signal to false, so that MM will not read data after this request if not needed
    wait(SC_ZERO_TIME);

    for (uint32_t i = 0; i < num_cache_levels; i++)
    {
      // wait till cache level will be ready
      wait_for_cache_level_ready(i);

      // if data was found in cache level, set select bits for multiplexers
      if (!hit && !L[i]->miss.read())
      {
        DEBUG_PRINT("MAIN: Hit in L[%d]: %s\n", i + 1, !L[i]->miss.read() ? "true" : "false");
        DEBUG_PRINT("MAIN: Read data in CACHE_LAYER[%d]: %u\n", i + 1, L[i]->data.read());

        hit = true;

        miss_mux_select.write(i);
        ready_mux_select.write(i);
        data_mux_select.write(i);
        wait_zero_cachetime();

        for (uint32_t j = 0; j < num_cache_levels; j++)
        {
          DEBUG_PRINT("MAIN: Setting stop signal for CACHE_LAYER[%d] to %s\n", j + 1, j == i ? "false" : "true");
          if (j != i) L[j]->stop = true; // stop waiting the latency in other cache levels
          else L[j]->idle = true; // set cache level with hit to idle in the next clock
        }
        main_memory.stop = true; // stop waiting the latency in main memory
        
        break;
      }
    }

    // if miss in all cache levels
    if (!hit)
    {
      DEBUG_PRINT("MAIN: Miss in all cache levels.\n");
      wait_for_main_memory_ready();

      std::vector<uint8_t> cacheline = get_cacheline_from_memory();

      // write cacheline to each cache level
      for (int i = 0; i < num_cache_levels; i++)
        L[i]->write_cacheline(addr.read(), cacheline);

      // set output data
      rdata.write(L[0]->extract_word(cacheline, addr.read() & (cacheline_size - 1)));
    }
  }

  // Helper function to get cacheline from main memory for read/write operations
  std::vector<uint8_t> get_cacheline_from_memory() {
    std::vector<uint8_t> cacheline;
    for (int i = 0; i < cacheline_size; i++)
    {
      cacheline.push_back(mem_cacheline[i].read());
    }
    return cacheline;
  }

  void set_main_memory_signals()
  {
    mem_addr.write(addr.read());
    mem_wdata.write(wdata.read());
    mem_r.write(r.read());
    mem_w.write(w.read());
  }

  void wait_for_main_memory_ready() {
    DEBUG_PRINT("MAIN: Waiting for main memory to be ready...\n");
    while (!mem_ready.read())
    {
      wait_zero_cachetime();
    }
  }

  void wait_for_cache_level_ready(const int i) {
    DEBUG_PRINT("MAIN: Waiting for cache level %d to be ready...\n", i + 1);
    while (!L[i]->ready.read())
    {
      wait_zero_cachetime();
    }
  }

  /**
 * @brief Handles a write request in the cache hierarchy.
 *
 * This function initiates a write operation by signaling the main memory and propagating the request
 * through all cache levels. It waits for each cache level to become ready and checks for cache hits.
 * For each cache level that misses, it writes the updated cacheline from main memory into that level.
 * The function ensures that the written data is reflected in both the cache hierarchy and main memory,
 * and updates multiplexer select signals to route the correct outputs after the operation.
 */
  void doWrite()
  {
    bool hit[num_cache_levels]; // hit[i] = true if cache level i was hit
    uint8_t indexL[num_cache_levels];

    w_mux_in.write(true); // setting write signal to true, so that MM will start writing data
    wait(); // wait for the next clock cycle so that memory and cache levels start processing the request
    mem_w.write(false); // setting write signal to false, so that MM will not write data after this request if not needed
    w_mux_in.write(false);
    wait(SC_ZERO_TIME);

    for (int i = 0; i < num_cache_levels; i++)
    {
      wait_for_cache_level_ready(i);

      if (!L[i]->miss.read())
      {
        miss_mux_select.write(i); // TODO: fix
        ready_mux_select.write(i);
        data_mux_select.write(i);
        wait(SC_ZERO_TIME);
      }
      
      hit[i] = !L[i]->miss.read();
    }
    DEBUG_PRINT("MAIN: Hit caches in write -> L[1]: %s, L[2]: %s, L[3]: %s\n", hit[0] ? "true" : "false", hit[1] ? "true" : "false", hit[2] ? "true" : "false");

    DEBUG_PRINT("MAIN: Waiting for main memory to be ready...\n");
    wait_for_main_memory_ready();

    DEBUG_PRINT("MAIN: Main memory is ready, writing data to cache levels...\n");

    std::vector<uint8_t> cacheline = get_cacheline_from_memory();

    // write data to each cache level, where it was miss
    for (int i = 0; i < num_cache_levels; i++)
      if (!hit[i]) L[i]->write_cacheline(addr.read(), cacheline);

    DEBUG_PRINT("MAIN: Data written to cache levels.\n");
  }

  // returns byte in cache-level: level, cache line: line_index, at position: index
  uint8_t get_cacheline_content(uint32_t level, uint32_t line_index, uint32_t index)
  {
    if (level <= 0 || level > num_cache_levels)
      throw std::runtime_error("Number of cache levels must be in range [1;3] in method get_cacheline_content.\n");
    return L[level - 1]->get_cacheline_content(line_index, index);
  }

  void set_memory(uint32_t address, uint32_t value)
  {
    main_memory.set(address, value);
  }
};

#endif // CACHE_HPP
