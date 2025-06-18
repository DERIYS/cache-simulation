#ifndef CACHE_HPP
#define CACHE_HPP

#include <list>
#include <systemc>
#include <systemc.h>
using namespace sc_core;

struct CacheLine {
  uint32_t tag;
  std::vector<uint32_t> data;
};

SC_MODULE(CACHE) {
  sc_in<uint32_t> addr, wdata;
  sc_in<bool> clk, r, w;

  sc_out<bool> hit, ready;
  sc_out<uint32_t> data;

  uint32_t latency, num_lines, cacheline_size;
  uint8_t mapping_strategy;

  std::vector<CacheLine> cache_memory;
  std::list<CacheLine> lru_track;

  // TODO: implement read functions
  bool read_direct_mapped();
  bool read_fully_associative();

  // TODO: implement write methods
  bool write_direct_mapped();
  bool write_fully_associative();

  // This function serves as a wrapper for other read methods that depends on the mapping strategy
  bool read() {
    if (mapping_strategy == 0) // direct-mapped
      return read_direct_mapped();

    if (mapping_strategy == 1) // fully-associative
      return read_fully_associative();

    throw std::runtime_error("Invalid mapping_strategy");
  }

  // This function serves as a wrapper for other write methods that depends on the mapping strategy
  bool write() {
    if (mapping_strategy == 0) // direct-mapped
      return write_direct_mapped();

    if (mapping_strategy == 1) // fully-associative
      return write_fully_associative();

    throw std::runtime_error("Invalid mapping_strategy");
  }

  void behaviour() {
    while (true) {
      wait();
      ready.write(false);
      if (r.read()) read();
      if (w.read()) write();
    }
  }

  SC_CTOR(HALF_ADDER);

  CACHE(sc_module_name name, uint32_t latency, uint32_t num_lines, uint32_t cacheline_size, uint8_t mapping_strategy)
    : sc_module(name), latency(latency), num_lines(num_lines), cacheline_size(cacheline_size), mapping_strategy(mapping_strategy)
  {
    cache_memory.resize(num_lines);

    SC_THREAD(behaviour);
    sensitive << clk.pos();
  }
};

#endif