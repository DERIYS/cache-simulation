#ifndef MAIN_MEMORY_HPP
#define MAIN_MEMORY_HPP

#include <systemc>
#include <map>
using namespace sc_core;

#define LATENCY 10

SC_MODULE(MAIN_MEMORY) {
  sc_in<bool> clk;

  sc_in<uint32_t> addr;
  sc_in<uint32_t> wdata;
  sc_in<bool> r;
  sc_in<bool> w;

  std::vector<sc_out<uint8_t>> cacheline;
  sc_out<bool> ready;

  std::map<uint32_t, uint8_t> memory;

  SC_CTOR(MAIN_MEMORY);
  MAIN_MEMORY(sc_module_name name, uint32_t cacheline_size):sc_module(name),cacheline(cacheline_size){
    SC_THREAD(behaviour);
    sensitive << clk.pos();
  } 

  void behaviour() {
    while(true) {
      wait();
      ready.write(false);
      if (r.read()) {

        doRead(w.read());
      }
      if (w.read()) {
        doWrite();
      }
    }
  }

  void doRead(bool dontSetReady) {
    ready.write(false);
    std::vector<uint8_t> result = getCacheLine(addr.read());

    for(int i = 0; i < LATENCY; i++) {
      wait();
    }
    for(int i=0;i<cacheline.size();i++){
      cacheline[i].write(result[i]);
    }
    if(!dontSetReady) {
      ready.write(true);
    }
  }

  void doWrite() {
    ready.write(false);
    set(addr.read(), wdata.read());

    for(int i = 0; i < LATENCY; i++) {
      wait();
    }
    ready.write(true);
  }

  uint32_t get(uint32_t address) {
    uint32_t result = 0;

    for (int i = 0; i < 4; i++) {
      uint8_t value = 0;
      if(memory.find(address + i) != memory.end()) {
        value = memory[address + i];
      }
      result |= value << (i * 8);
    }

    return result;
  }

  void print(){
    for(int i=0;i<memory.size();i++){
      if(i%4==0) std::cout << "mem[0x" << std::hex << (i) << "] ";
      std::cout << (int)memory[i]<<" " ;
      if((i+1)%4==0) std::cout<< std::endl;
    }
  }

  void set(uint32_t address, uint32_t value) {
    for (int i = 0; i < 4; i++) {
      memory[address + i] = (value >> (i * 8)) & 0xFF;
      if(address + i == UINT32_MAX) {
        break;
      }
    }
    std::vector<uint8_t> result = getCacheLine(address);
    for(int i=0;i<cacheline.size();i++){
      cacheline[i].write(result[i]);
    }

  }

  //get whole cache line wenn cache miss
  std::vector<uint8_t> getCacheLine(uint32_t address){
    uint32_t start = address & ~(cacheline.size() - 1);
    std::vector<uint8_t> result(cacheline.size());
    for(int i=0;i<cacheline.size();i++){
      uint8_t value = 0;
      if(memory.find(start + i) != memory.end()) {
        value = memory[start + i];
      }
      result[i]=value;
    }
    return result;
  }
  

};

#endif // MAIN_MEMORY_HPP