#ifndef CACHE_HPP
#define CACHE_HPP

#include "cache_layer.hpp"
#include "multiplexer.hpp"
#include "main_memory.hpp"
#include <systemc>
#include <map>
#include <vector>
#include <memory>

using namespace sc_core;

SC_MODULE(CACHE) {
  //inputs
  sc_in<bool> clk;

  sc_in<uint32_t> addr, wdata;
  sc_in<bool> r, w;

  sc_in<bool> mem_ready;
  sc_in<uint32_t> mem_rdata;

  //outputs
  sc_out<uint32_t> rdata;
  sc_out<bool> ready,miss;

  sc_out<uint32_t> mem_addr,mem_wdata;
  sc_out<bool> mem_r,mem_w;

  //modules
  std::vector<std::unique_ptr<CACHE_LAYER>> L;
  MAIN_MEMORY main_memory;
  MULTIPLEXER_I32 cacheData;
  
  //parameters
  uint8_t numCahceLevels;
  uint32_t cachelineSize, numLinesL1, numLinesL2, numLinesL3;
  uint32_t latencyCacheL1,  latencyCacheL2,  latencyCacheL3;
  uint8_t mappingStrategy;

  //signals
  sc_signal<uint32_t> addrL[3],wdataL[3],addrMem,wdataMem;
  sc_signal<bool> rL[3],wL[3],rMem,wMem;
  sc_signal<uint8_t> sL[3];

  SC_CTOR(CACHE);

  CACHE(sc_module_name name, uint8_t numCahceLevels, uint32_t cachelineSize,uint32_t numLinesL1,uint32_t numLinesL2,
    uint32_t numLinesL3, uint32_t latencyCacheL1, uint32_t latencyCacheL2, uint32_t latencyCacheL3, uint8_t mappingStrategy)
    :sc_module(name),
    L(numCahceLevels),
    numCahceLevels(numCahceLevels),
    cachelineSize(cachelineSize),
    numLinesL1(numLinesL1),numLinesL2(numLinesL2),numLinesL3(numLinesL3),
    latencyCacheL1(latencyCacheL1),latencyCacheL2(latencyCacheL2),latencyCacheL3(latencyCacheL3),
    mappingStrategy(mappingStrategy),
    cacheData("cacheData",numCahceLevels,1),
    main_memory("main_memory")
    { 
      switch (numCahceLevels)
      {
        case 3:
          L[2]=(std::make_unique<CACHE_LAYER>("L3",latencyCacheL3,numLinesL3,cachelineSize,mappingStrategy));
        case 2:
          L[1]=(std::make_unique<CACHE_LAYER>("L2",latencyCacheL2,numLinesL2,cachelineSize,mappingStrategy));
        case 1:
          L[0]=(std::make_unique<CACHE_LAYER>("L1",latencyCacheL1,numLinesL1,cachelineSize,mappingStrategy));
          break;
        default:
          throw std::runtime_error("Number of Cache Levels must be in range [1;3].\n");
          break;
      }

      for(int i=0;i<numCahceLevels;i++){
        L[i]->clk(clk);
        L[i]->addr(addrL[i]);
        addr(addrL[i]);
        L[i]->wdata(wdataL[i]);
        wdata(wdataL[i]);
        L[i]->r(rL[i]);
        r(rL[i]);
        L[i]->w(wL[i]);
        w(wL[i]);
        cacheData.in[i](L[i]->data);
        rdata(cacheData.out[0]);
        sL[i].write(i);
      }
      
      main_memory.clk(clk);
      main_memory.addr(addrMem);
      addr(addrMem);
      main_memory.wdata(wdataMem);
      wdata(wdataMem);
      main_memory.r(rMem);
      main_memory.w(wMem);

    SC_THREAD(behaviour);
    sensitive << clk.pos();
  }

  void behaviour() {
    while(true) {
        wait();
      if (r.read()) {
        doRead(w.read());
      }
      if (w.read()) {
        doWrite();
      }
    }
  }

  void doRead(bool dontSetReady) {
    bool hit=false;
    uint8_t indexL=0;
    for(int i=0;i<numCahceLevels;i++){
        while(!L[i]->ready.read())
            wait();
      if( L[i]->hit.read()){
        cacheData.select(sL[i]);//choose select bit based on cache-hit from L1 to L3, so rdata will be provided  
        wait(SC_ZERO_TIME);
        hit=true;
        indexL=i;
        break;
      }
    }
    if(hit){
        for(int i = 0; i <L[indexL]->latency; i++) {
            wait();//wait letency for cache, where data was found
        }
    }else{
        //cache miss, read data from main_memory and write it to all 3 caches 
        miss.write(true);
        //wait till main_memory finish to read the data
        do{
            wait();
        }while(!mem_ready);
        //memory latecny will be waited in main_memory function
        for(int i=0;i<numCahceLevels;i++){
            L[i]->write_data_from_main_memory(NULL,mem_rdata.read());//i dont think there should be argument for addres, bc you dont need adress for fifo or lifo
        }
    }
    if(!dontSetReady)
        ready.write(true);
  }

  void doWrite() {
    bool hit=false;
    uint8_t indexL=0;
    for(int i=0;i<numCahceLevels;i++){
        while(!L[i]->ready.read())
            wait();
        if( L[i]->hit.read()){
            hit=true;
            indexL=i;
            break;
      }
    }
    if(!hit){
        miss.write(true);
        indexL=255;
    }
    //the data must be written in rest caches and in main_memory
    for(int i=0;i<numCahceLevels;i++)
        if(i!=indexL)
            L[i]->write_data_from_main_memory(NULL,rdata.read());//rdata was set in cache_layer 
    main_memory.set(addr.read(),rdata.read());
    ready.write(true);
  }

  

  //returns byte in cache-level: level, cache line: lineIndex, at position: index
  uint8_t getCacheLineContent(uint32_t level, uint32_t lineIndex, uint32_t index){
    if(level<=0 || level>numCahceLevels)
        throw std::runtime_error("Number of Cache Levels must be in range [1;3] in method getCacheLineContent.\n");
    return L[level-1]->getCacheLineContent(lineIndex,index);
  }

 
};

#endif // CACHE_HPP