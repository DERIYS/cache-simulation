#ifndef CACHE_HPP
#define CACHE_HPP

#include "cache_layer.hpp"
#include "multiplexer.hpp"
#include "main_memory.hpp"
#include <systemc>
#include <map>
#include <vector>
#include <memory>

#define MAIN_MEMORY_LATENCY 10

using namespace sc_core;

SC_MODULE(CACHE) {
  bool test;
  //inputs
  sc_in<bool> clk;

  sc_in<uint32_t> addr, wdata;
  sc_in<bool> r, w;

  sc_in<bool> mem_ready;
  std::vector<sc_in<uint8_t>> mem_cacheline;

  //outputs
  sc_out<uint32_t> rdata;
  sc_out<bool> ready,miss;

  sc_out<uint32_t> mem_addr,mem_wdata;
  sc_out<bool> mem_r,mem_w;

  //modules
  std::vector<std::unique_ptr<CACHE_LAYER>> L;
  MAIN_MEMORY main_memory;
  MULTIPLEXER_BOOLEAN cache_miss,cache_ready, r_mux,w_mux;
  MULTIPLEXER_I32 cache_data,addr_mux,wdata_mux;
  
  //parameters
  uint8_t num_cahce_levels;
  uint32_t cacheline_size, num_lines_L1, num_lines_L2, num_lines_L3;
  uint32_t latency_cache_L1,  latency_cache_L2,  latency_cache_L3;
  uint8_t mapping_strategy;

  //signals
  sc_signal<uint32_t> addr_mux_in,addr_mux_out[3],wdata_mux_in,wdata_mux_out[3];
  sc_signal<bool> r_mux_in,r_mux_out[3],w_mux_in,w_mux_out[3];

  sc_signal<uint32_t> mem_addr_sig,mem_wdata_sig;
  std::vector<sc_signal<uint8_t>> mem_cacheline_sig;
  sc_signal<bool> mem_r_sig,mem_w_sig,mem_ready_sig;
  //multiplexer signals
  sc_signal<uint8_t> data_mux_select,miss_mux_select,ready_mux_select,select_zero;
  sc_signal<uint32_t> cache_data_in[3],cache_data_out;
  sc_signal<bool> cache_miss_in[3],cache_miss_out,cache_ready_in[3],cache_ready_out;


  SC_CTOR(CACHE);

  CACHE(sc_module_name name, uint8_t num_cache_levels, uint32_t cacheline_size,uint32_t num_lines_L1,uint32_t num_lines_L2,
    uint32_t num_lines_L3, uint32_t latency_cache_L1, uint32_t latency_cache_L2, uint32_t latency_cache_L3, uint8_t mapping_strategy)
    :sc_module(name),
    L(num_cache_levels),
    num_cahce_levels(num_cache_levels),
    cacheline_size(cacheline_size),
    num_lines_L1(num_lines_L1),num_lines_L2(num_lines_L2),num_lines_L3(num_lines_L3),
    latency_cache_L1(latency_cache_L1),latency_cache_L2(latency_cache_L2),latency_cache_L3(latency_cache_L3),
    mapping_strategy(mapping_strategy),
    cache_data("cacheData",num_cache_levels,1),
    cache_miss("cacheMiss",num_cache_levels,1),
    cache_ready("cacheReady",num_cache_levels,1),
    r_mux("rMil",1,num_cache_levels),
    w_mux("wMul",1,num_cache_levels),
    addr_mux("addrMul",1,num_cache_levels),
    wdata_mux("wdataMul",1,num_cache_levels),
    main_memory("main_memory",cacheline_size),
    mem_cacheline_sig(cacheline_size),
    mem_cacheline(cacheline_size)
    { 
      switch (num_cache_levels)
      {
        case 3:
          L[2]=(std::make_unique<CACHE_LAYER>("L3",latency_cache_L3,num_lines_L3,cacheline_size,mapping_strategy));
        case 2:
          L[1]=(std::make_unique<CACHE_LAYER>("L2",latency_cache_L2,num_lines_L2,cacheline_size,mapping_strategy));
        case 1:
          L[0]=(std::make_unique<CACHE_LAYER>("L1",latency_cache_L1,num_lines_L1,cacheline_size,mapping_strategy));
          break;
        default:
          throw std::runtime_error("Number of Cache Levels must be in range [1;3].\n");
          break;
      }
      //set up mux for input data. 1 input, numCacheLevels output. each output for each cache level. select-bit everywhere = 0
      //input data will be writen to mux in behavior()
      select_zero.write(0);
      addr_mux.in[0](addr_mux_in); addr_mux.select(select_zero);
      wdata_mux.in[0](wdata_mux_in); wdata_mux.select(select_zero);
      r_mux.in[0](r_mux_in); r_mux.select(select_zero);
      w_mux.in[0](w_mux_in); w_mux.select(select_zero);

      //connect ports of each cache level
      for(int i=0;i<num_cache_levels;i++){
        L[i]->clk(clk);
        //use outputs from mux as input data
        L[i]->addr(addr_mux_out[i]); addr_mux.out[i](addr_mux_out[i]);
        L[i]->wdata(wdata_mux_out[i]); wdata_mux.out[i](wdata_mux_out[i]);
        L[i]->r(r_mux_out[i]); r_mux.out[i](r_mux_out[i]);
        L[i]->w(w_mux_out[i]); w_mux.out[i](w_mux_out[i]);
        //conncecnt to input of mux. numCacheLevels inputs, 1 output
        L[i]->data(cache_data_in[i]);cache_data.in[i](cache_data_in[i]);
        L[i]->miss(cache_miss_in[i]);cache_miss.in[i](cache_miss_in[i]);
        L[i]->ready(cache_ready_in[i]);cache_ready.in[i](cache_ready_in[i]);
      }
      
      //connect output from mux, to get data from one of the cache levels
      cache_data.out[0](cache_data_out);cache_data.select(data_mux_select);
      cache_miss.out[0](cache_miss_out);  cache_miss.select(miss_mux_select);
      cache_ready.out[0](cache_ready_out); cache_ready.select(ready_mux_select);

      //connect inputs/outputs for main_memory module
      main_memory.clk(clk);
      main_memory.addr(mem_addr_sig);mem_addr(mem_addr_sig);
      main_memory.wdata(mem_wdata_sig);mem_wdata(mem_wdata_sig);
      main_memory.r(mem_r_sig); mem_r(mem_r_sig);
      main_memory.w(mem_w_sig); mem_w(mem_w_sig);
      for(int i=0;i<cacheline_size;i++){
        main_memory.cacheline[i](mem_cacheline_sig[i]);
        mem_cacheline[i](mem_cacheline_sig[i]);
      }
      main_memory.ready(mem_ready_sig); mem_ready(mem_ready_sig);

    SC_THREAD(behaviour);
    sensitive << clk.pos();
  }

  void print_caches(){
    for(int i = 0; i < num_cahce_levels; i++){
        L[i]->print(i+1);
    }std::cout<<"\n";
}

  void behaviour() {
    while(true) {
        wait();
        ready.write(false);
        miss.write(false);
        //set input signals in multiplexer, which forward them to each cache level
        w_mux_in.write(w.read());
        r_mux_in.write(r.read());
        wdata_mux_in.write(wdata.read());
        addr_mux_in.write(addr.read()); 
        wait(SC_ZERO_TIME);
        if (r.read()) {
            doRead(w.read());
        }
        if (w.read()) {
            doWrite();
        }
    }
  }

  //waiting till each cache level from low to high will be ready
  //and check if there is a hit/miss. if hit - wait cache latency and return rdata
  //miss - set input signals for main_memory, wait till ready, write readed cacheline in each cache level and return it
  void doRead(bool dontSetReady) {
    bool hit=false;
    uint8_t indexL=0;
    for(int i=0;i<num_cahce_levels;i++){
      //wait till cache level will be ready with reading
        while(!L[i]->ready.read()){
            wait();
        }
        //if data was find in cache level->defines select bits for multiplexers
        if( !L[i]->miss.read()){
          miss_mux_select.write(i);
          ready_mux_select.write(i);
          data_mux_select.write(i);
          wait(SC_ZERO_TIME);
          hit=true;
          indexL=i;
          if(test) std::cout<<"hit cache in read: L[ "<<i<<" ]: "<<hit<<"\n";
          break;
        }
    }
    //if it was hit at some of cache levels
    if(hit){
      rdata.write(cache_data.out[0].read());
      for(int i = 0; i <L[indexL]->latency; i++) {
          wait();//wait letency for cache, where data was found
      }
    }else{
        if(test) std::cout<<"miss cache in read \n";
        mem_addr.write(addr.read());
        mem_wdata.write(wdata.read());
        mem_r.write(r.read());mem_w.write(w.read());
        //cache miss, read data from main_memory and write it to all 3 caches 
         mem_r.write(true);//start reading in main_memory for one cycle
         wait();
         mem_r.write(false);
        //wait till main_memory finish to read the data
        while(!mem_ready.read()){
          wait();
        }
        //memory latecny will be waited in main_memory function
        std::vector<uint8_t> t;
        for(int i=0;i<cacheline_size;i++){
          t.push_back(mem_cacheline[i].read());
        }
        for(int i=0;i<num_cahce_levels;i++){
            L[i]->write_cacheline(addr.read(),t);
        }
        rdata.write( L[0]->extract_word(t,addr.read() & (cacheline_size-1)));
    }
    if(!dontSetReady){
         ready.write(cache_ready.out[0].read());
    }
     miss.write(cache_miss.out[0].read());
  }

  void doWrite() {
    bool hit[num_cahce_levels];
    uint8_t indexL[num_cahce_levels];
    for(int i=0;i<num_cahce_levels;i++){
        while(!L[i]->ready.read())
            wait();
        if( !L[i]->miss.read()){
            hit[i]=true;
            miss_mux_select.write(i);
            ready_mux_select.write(i);
            data_mux_select.write(i);
        }else
            hit[i]=false;
    }
    if(test) std::cout<<"hit caches in write: L[1]: "<<hit[0]<<", L[2]: "<<hit[1]<<", L[3]: "<<hit[2]<<"\n";
    mem_addr.write(addr.read());
    mem_wdata.write(wdata.read());
    mem_r.write(r.read());mem_w.write(w.read());
    //the data must be written in rest caches and in main_memory
    while(!mem_ready.read())
      wait();
    mem_w.write(false);
    std::vector<uint8_t> t;
    for(int i=0;i<cacheline_size;i++){
      t.push_back(mem_cacheline[i].read());
    }
    for(int i=0;i<num_cahce_levels;i++)
        if(!hit[i]){
            L[i]->write_cacheline(addr.read(),t); 
        }
    ready.write(cache_ready.out[0].read());
    miss.write(cache_miss.out[0].read());
  }

  

  //returns byte in cache-level: level, cache line: lineIndex, at position: index
  uint8_t getCacheLineContent(uint32_t level, uint32_t lineIndex, uint32_t index){
    if(level<=0 || level>num_cahce_levels)
        throw std::runtime_error("Number of Cache Levels must be in range [1;3] in method getCacheLineContent.\n");
    return L[level-1]->getCacheLineContent(lineIndex,index);
  }

  void set_memory(uint32_t address, uint32_t value){
    main_memory.set(address,value);
  }
 
};

#endif // CACHE_HPP
