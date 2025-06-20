#ifndef CACHE_HPP
#define CACHE_HPP

#include <list>
#include <systemc>
#include <systemc.h>
#include <unordered_map>
using namespace sc_core;

struct CacheLine {
  uint32_t tag;
  bool valid;
  std::vector<uint8_t> data;
};

#define INVALID_ADDRESS_EXCEPTION std::runtime_error("InvalidAddressException: Invalid offset for 4-byte access");

SC_MODULE(CACHE_LAYER) {
  sc_in<uint32_t> addr, wdata;
  sc_in<bool> clk, r, w;

  sc_out<bool> hit, ready;
  sc_out<uint32_t> data;

  uint32_t latency, num_lines, cacheline_size;
  uint8_t mapping_strategy;

  // Number of actually occupied cache lines.
  // Used only with fully-associative mapping strategy to pick an index for a new write data or determining if the memory is full
  uint32_t size;

  std::vector<CacheLine> cache_memory;
  std::list<uint32_t> lru_list; // Indexes of cache_memory in LRU order (head: MRU, tail: LRU)
  std::unordered_map<uint32_t, std::list<uint32_t>::iterator> lru_map; // Maps tag to lru_list node

  // Helper function to set offset, (index), and tag values
  void set_offset_index_tag(uint32_t address, uint32_t &offset, uint32_t *index, uint32_t &tag) const {
    const uint32_t offset_bits = __builtin_ctz(cacheline_size);
    const uint32_t index_bits = __builtin_ctz(num_lines);

    offset = address & (cacheline_size - 1);
    if (index) *index = (address >> offset_bits) & (num_lines - 1);
    tag = (mapping_strategy == 0) ? address >> (offset_bits + index_bits): address >> (offset_bits);
  }

  // Helper function to extract a word from a cacheline with the offset
  static uint32_t extract_word(const std::vector<uint8_t> &cacheline, const uint32_t offset) {
    return (cacheline[offset + 3] << 24) |
         (cacheline[offset + 2] << 16) |
         (cacheline[offset + 1] << 8) |
         cacheline[offset];
  }

  // Helper function to write data to a cacheline with the offset
  static void write_data(std::vector<uint8_t> cacheline, const uint32_t wdata_val, const uint32_t offset) {
    cacheline[offset] = wdata_val & 0xFF;
    cacheline[offset + 1] = (wdata_val >> 8) & 0xFF;
    cacheline[offset + 2] = (wdata_val >> 16) & 0xFF;
    cacheline[offset + 3] = (wdata_val >> 24) & 0xFF;
  }

  // This function should be called from the main cache module to write retrieved data from main memory after miss
  void write_data_from_main_memory(const uint32_t address, const uint32_t wdata) {
    // TODO: implement the function
  }

  // Called with every clock tick if the layer's mapping strategy is direct-mapped
  void access_direct_mapped() {
    uint32_t offset, index, tag;
    set_offset_index_tag(addr.read(), offset, &index, tag);

    if (offset + 3 >= cacheline_size) {
      throw INVALID_ADDRESS_EXCEPTION;
    }

    if (cache_memory[index].valid && cache_memory[index].tag == tag) { // Cache hit
      hit.write(true);
      if (r.read())
        data.write(extract_word(cache_memory[index].data, offset));
      if (w.read())
        write_data(cache_memory[index].data, wdata.read(), offset);
      return;
    }
    hit.write(false);
  }

  // Called with every clock tick if the layer's mapping strategy is fully-associative
  void access_fully_associative() {
    uint32_t offset, tag;
    set_offset_index_tag(addr.read(), offset, nullptr, tag);

    if (offset + 3 >= cacheline_size) {
      throw INVALID_ADDRESS_EXCEPTION;
    }

    auto it = lru_map.find(tag);
    if (it != lru_map.end()) { // tag in lru_map -> cache hit
      // Move the according node in lru_list to the beginning and update the map value
      uint32_t index = *it->second; // this extracts the index of the needed cacheline from the map
      lru_list.erase(it->second); // erasing the node associated with the tag from lru_list
      lru_list.push_front(index); // pushing it to the beginning
      lru_map[tag] = lru_list.begin(); // updating the map

      hit.write(true);
      if (r.read())
        data.write(extract_word(cache_memory[index].data, offset));
      if (w.read())
        write_data(cache_memory[index].data, wdata.read(), offset);
      return;
    }
    hit.write(false);
  }

  void behaviour() {
    while (true) {
      // Latency will be handled in the main cache.hpp systemc module as it is impossible to communicate with other cache levels inside this module
      // to ensure that the right cache layer's latency will be waited (L1 level latency if found in L1, L2 if in L2 etc.)
      wait();
      ready.write(false);
      if (mapping_strategy == 0) access_direct_mapped();
      else if (mapping_strategy == 1) access_fully_associative();
      else throw std::runtime_error("Invalid mapping_strategy");
      ready.write(true);
    }
  }

  SC_CTOR(CACHE_LAYER);

  CACHE_LAYER(const sc_module_name &name, const uint32_t latency, const uint32_t num_lines, uint32_t cacheline_size, uint8_t mapping_strategy)
    : sc_module(name), latency(latency), num_lines(num_lines), cacheline_size(cacheline_size), mapping_strategy(mapping_strategy)
  {
    if (__builtin_popcount(cacheline_size) != 1 || __builtin_popcount(num_lines) != 1) {
      throw std::runtime_error("InvalidArgumentException: cacheline_size and num_lines must be powers of 2");
    }

    if (mapping_strategy == 1) size = 0;
    cache_memory.resize(num_lines, {0, false, std::vector<uint8_t>(cacheline_size)});
    SC_THREAD(behaviour);
    sensitive << clk.pos();
  }

  // For test purposes
  void set_memory(const std::vector<CacheLine>& cache_memory, const std::list<uint32_t>& lru_list, std::unordered_map<uint32_t, std::list<uint32_t>::iterator> lru_map) {
    this->cache_memory = cache_memory;
    this->lru_list = lru_list;
    this->lru_map = lru_map;
  }
};

#endif