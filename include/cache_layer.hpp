#ifndef CACHE_LAYER_HPP
#define CACHE_LAYER_HPP

#include <list>
#include <systemc>
#include <systemc.h>
#include <unordered_map>
using namespace sc_core;

struct CacheLine
{
  uint32_t tag;
  bool valid;
  std::vector<uint8_t> data;
};

constexpr uint8_t DIRECT_MAPPED = 0;
constexpr uint8_t FULLY_ASSOCIATIVE = 1;

SC_MODULE(CACHE_LAYER)
{
  sc_in<uint32_t> addr, wdata;
  sc_in<bool> clk, r, w;

  sc_out<bool> hit, ready;
  sc_out<uint32_t> data;
  sc_out<bool> error_occurred;

  uint32_t latency, num_lines, cacheline_size;
  uint8_t mapping_strategy;

  bool test_mode = false; // If true, the cache is in test mode and does not throw exceptions
  bool error = false; // If true, the cache has encountered an error

  // Number of actually occupied cache lines.
  // Used only with fully-associative mapping strategy to pick an index for new cacheline or determining if the memory is full
  uint32_t size;

  std::vector<CacheLine> cache_memory;
  std::list<uint32_t> lru_list;                                        // Indexes of cache_memory in LRU order (head: MRU, tail: LRU)
  std::unordered_map<uint32_t, std::list<uint32_t>::iterator> lru_map; // Maps tag to lru_list node


  //just for testing, can be deleted spater 
  void print(int l){
    std::cout<<"L: "<<l<<"\n";
    for(int i=0;i<num_lines;i++){
      for(int j=0;j<cacheline_size;j++){
        std::cout << " " << std::hex <<(int)cache_memory[i].data[j] ;
        if((j+1)%4==0) cout<<"|";
      }
     std::cout<<"\n";
    }
  }

  // Helper function to set offset, (index), and tag values
  void set_offset_index_tag(const uint32_t address, uint32_t *offset, uint32_t *index, uint32_t &tag)
  {
    const uint32_t offset_bits = __builtin_ctz(cacheline_size);
    const uint32_t index_bits = __builtin_ctz(num_lines);

    if (offset)
    {
      *offset = address & (cacheline_size - 1);
      if (*offset + 3 >= cacheline_size)
      {
        error = true;
        if (!test_mode) throw std::runtime_error("InvalidAddressException: Invalid offset for 4-byte access in set_offset_index_tag");
        return;
      }
    }

    if (index)
      *index = (address >> offset_bits) & (num_lines - 1);

    tag = (mapping_strategy == DIRECT_MAPPED) ? address >> (offset_bits + index_bits) : address >> (offset_bits);
  }

  // Helper function to extract a word from a cacheline with the offset
  static uint32_t extract_word(const std::vector<uint8_t> &cacheline, const uint32_t offset)
  {
    return (cacheline[offset + 3] << 24) |
           (cacheline[offset + 2] << 16) |
           (cacheline[offset + 1] << 8) |
           cacheline[offset];
  }

  // Helper function for main cache module
  uint8_t getCacheLineContent(uint32_t line_index, uint32_t index)
  {
    if (line_index >= cache_memory.size())
    {
      error = true;
      if (!test_mode) throw std::runtime_error("Line index out of bounds in getCacheLineContent method.\n");
      return 0; // Return 0 or some default value to avoid undefined behavior
    }
    if (index >= cache_memory[line_index].data.size())
    {
      error = true;
      if (!test_mode) throw std::runtime_error("Data index out of bounds in getCacheLineContent method.\n");
      return 0; // Return 0 or some default value to avoid undefined behavior
    }
    return cache_memory[line_index].data[index];
  }

  // Helper function to write data to a cacheline with the offset
  void write_data(std::vector<uint8_t> & cacheline, const uint32_t wdata_val, const uint32_t offset)
  {
    if (offset + 3 >= cacheline.size())
    {
      error = true;
      if (!test_mode) throw std::runtime_error("InvalidAddressException: Invalid offset for 4-byte access in write_data");
    }

    cacheline[offset] = wdata_val & 0xFF;
    cacheline[offset + 1] = (wdata_val >> 8) & 0xFF;
    cacheline[offset + 2] = (wdata_val >> 16) & 0xFF;
    cacheline[offset + 3] = (wdata_val >> 24) & 0xFF;
  }

  // This function should be called from the main cache module to write retrieved data from main memory after miss
  void write_cacheline(uint32_t addr, const std::vector<uint8_t> &mem_data)
  {
    uint32_t tag;
    set_offset_index_tag(addr, nullptr, nullptr, tag);

    uint32_t index;

    if (mapping_strategy == DIRECT_MAPPED)
    { // Direct-mapped
      uint32_t offset, direct_index;
      set_offset_index_tag(addr, nullptr, &direct_index, tag);
      cache_memory[direct_index] = {tag, true, mem_data};
    }
    else if (mapping_strategy == FULLY_ASSOCIATIVE)
    { // Fully-associative
      if (size < num_lines)
      {
        index = size++;
      }
      else
      {
        index = lru_list.back();
        lru_list.pop_back();
        lru_map.erase(cache_memory[index].tag);
      }
      cache_memory[index] = {tag, true, mem_data};
      lru_list.push_front(index);
      lru_map[tag] = lru_list.begin();
    }
    else
    {
      error = true;
      if (!test_mode) throw std::runtime_error("Invalid mapping_strategy in write_data_from_main_memory");
      return;
    }
  }

  // Called with every clock tick if the layer's mapping strategy is direct-mapped
  void access_direct_mapped()
  {
    uint32_t offset, index, tag;
    set_offset_index_tag(addr.read(), &offset, &index, tag);

    if (cache_memory[index].valid && cache_memory[index].tag == tag)
    { // Cache hit
      hit.write(true);
      if (r.read())
        data.write(extract_word(cache_memory[index].data, offset));
      if (w.read())
        write_data(cache_memory[index].data, wdata.read(), offset);
      return;
    }
    // else {
    //   cout << "Cache miss at index: " << index << ", tag: " << std::hex << tag << std::dec << endl;
    //   cout << cache_memory[index].valid << " " << hex << cache_memory[index].tag << dec << endl;
    // }
    hit.write(false);
  }

  // Called with every clock tick if the layer's mapping strategy is fully-associative
  void access_fully_associative()
  {
    uint32_t offset, tag;
    set_offset_index_tag(addr.read(), &offset, nullptr, tag);

    auto it = lru_map.find(tag);
    if (it != lru_map.end())
    { // tag in lru_map -> cache hit
      // Move the according node in lru_list to the beginning and update the map value
      uint32_t index = *it->second;    // this extracts the index of the needed cacheline from the map
      lru_list.erase(it->second);      // erasing the node associated with the tag from lru_list
      lru_list.push_front(index);      // pushing it to the beginning
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

  void behaviour()
  {
    while (true)
    {
      // Latency will be handled in the main cache.hpp systemc module as it is impossible to communicate with other cache levels inside this module
      // to ensure that the right cache layer's latency will be waited (L1 level latency if found in L1, L2 if in L2 etc.)
      wait();
      error = false; // Reset error state at the beginning of each clock cycle
      if (r.read() || w.read())
      {
        ready.write(false);
        if (mapping_strategy == DIRECT_MAPPED)
          access_direct_mapped();
        else if (mapping_strategy == FULLY_ASSOCIATIVE)
          access_fully_associative();
        else
        {
          error = true;
          if (!test_mode) throw std::runtime_error("Invalid mapping_strategy");
          return;
        }
        error_occurred.write(error);
        ready.write(true);
      }
    }
  }

  SC_CTOR(CACHE_LAYER);

  CACHE_LAYER(const sc_module_name &name, const uint32_t latency, const uint32_t num_lines, uint32_t cacheline_size, uint8_t mapping_strategy)
      : sc_module(name), latency(latency), num_lines(num_lines), cacheline_size(cacheline_size), mapping_strategy(mapping_strategy)
  {
    if (__builtin_popcount(cacheline_size) != 1 || __builtin_popcount(num_lines) != 1)
    {
      if (!test_mode) throw std::runtime_error("InvalidArgumentException: cacheline_size and num_lines must be powers of 2");
      return;
    }

    if (mapping_strategy == FULLY_ASSOCIATIVE)
      size = 0;
    cache_memory.resize(num_lines, {0, false, std::vector<uint8_t>(cacheline_size)});
    SC_THREAD(behaviour);
    sensitive << clk.pos();
  }

  // For test purposes
  void set_memory(const std::vector<CacheLine> &cache_memory, const std::list<uint32_t> &lru_list, std::unordered_map<uint32_t, std::list<uint32_t>::iterator> lru_map)
  {
    this->cache_memory = cache_memory;
    this->lru_list = lru_list;
    this->lru_map = lru_map;
  }
};

#endif