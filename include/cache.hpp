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
  sc_in<uint32_t> mem_rdata;

  //outputs
  sc_out<uint32_t> rdata;
  sc_out<bool> ready,miss;

  sc_out<uint32_t> mem_addr,mem_wdata;
  sc_out<bool> mem_r,mem_w;

  //modules
  std::vector<std::unique_ptr<CACHE_LAYER>> L;
  MAIN_MEMORY main_memory;
  MULTIPLEXER_BOOLEAN cacheHit,cacheReady, rMul,wMul;
  MULTIPLEXER_I32 cacheData,addrMul,wdataMul;
  
  //parameters
  uint8_t numCahceLevels;
  uint32_t cachelineSize, numLinesL1, numLinesL2, numLinesL3;
  uint32_t latencyCacheL1,  latencyCacheL2,  latencyCacheL3;
  uint8_t mappingStrategy;

  //signals
  sc_signal<uint32_t> addrMulIn,addrMulOut[3],wdataMulIn,wdataMulOut[3];
  sc_signal<bool> rMulIn,rMulOut[3],wMulIn,wMulOut[3];

  sc_signal<uint32_t> addrMem,wdataMem,rdataMem;
  sc_signal<bool> rMem,wMem,readyMem;
  //multiplexer signals
  sc_signal<uint8_t> selectData,selectHit,selectReady,selectMux;
  sc_signal<uint32_t> cacheDataIn[3],cacheDataOut;
  sc_signal<bool> cacheHitIn[3],cacheHitOut,cacheReadyIn[3],cacheReadyOut;


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
    cacheHit("cacheHit",numCahceLevels,1),
    cacheReady("cacheReady",numCahceLevels,1),
    rMul("rMil",1,numCahceLevels),
    wMul("wMul",1,numCahceLevels),
    addrMul("addrMul",1,numCahceLevels),
    wdataMul("wdataMul",1,numCahceLevels),
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
      //set up mux for input data. 1 input, numCacheLevels output. each output for each cache level. select-bit everywhere = 0
      //input data will be writen to mux in behavior()
      selectMux.write(0);
      addrMul.select(selectMux);wdataMul.select(selectMux);rMul.select(selectMux);wMul.select(selectMux);
      addrMul.in[0](addrMulIn);
      wdataMul.in[0](wdataMulIn);
      rMul.in[0](rMulIn);
      wMul.in[0](wMulIn);

      //connect ports of each cache level
      for(int i=0;i<numCahceLevels;i++){
        L[i]->clk(clk);
        //use outputs from mux as input data
        L[i]->addr(addrMulOut[i]); addrMul.out[i](addrMulOut[i]);
        L[i]->wdata(wdataMulOut[i]); wdataMul.out[i](wdataMulOut[i]);
        L[i]->r(rMulOut[i]); rMul.out[i](rMulOut[i]);
        L[i]->w(wMulOut[i]); wMul.out[i](wMulOut[i]);
        //conncecnt to input of mux. numCacheLevels inputs, 1 output
        L[i]->data(cacheDataIn[i]);cacheData.in[i](cacheDataIn[i]);
        L[i]->hit(cacheHitIn[i]);cacheHit.in[i](cacheHitIn[i]);
        L[i]->ready(cacheReadyIn[i]);cacheReady.in[i](cacheReadyIn[i]);
      }
      
      //connect output from mux, to get data from one of the cache levels
      cacheData.out[0](cacheDataOut); cacheData.select(selectData);
      cacheHit.out[0](cacheHitOut);   cacheHit.select(selectHit);
      cacheReady.out[0](cacheReadyOut); cacheReady.select(selectReady);

      //connect inputs/outputs for main_memory module
      main_memory.clk(clk);
      main_memory.addr(addrMem);mem_addr(addrMem);
      main_memory.wdata(wdataMem);mem_wdata(wdataMem);
      main_memory.r(rMem); mem_r(rMem);
      main_memory.w(wMem); mem_w(wMem);
      main_memory.rdata(rdataMem); mem_rdata(rdataMem);
      main_memory.ready(readyMem); mem_ready(readyMem);

    SC_THREAD(behaviour);
    sensitive << clk.pos();
  }

  void behaviour() {
    while(true) {
        wait();
        if(!test){
          miss.write(false);
          ready.write(false);
        }
        wMulIn.write(w.read());
        rMulIn.write(r.read());
        wdataMulIn.write(wdata.read());
        addrMulIn.write(addr.read()); 
        wait(SC_ZERO_TIME);
        if (r.read()) {
            doRead(w.read());
        }
        if (w.read()) {
            doWrite();
        }
    }
  }

  void doRead(bool dontSetReady) {
    if(test){
          miss.write(false);
          ready.write(false);
        }
    bool hit=false;
    uint8_t indexL=0;
    for(int i=0;i<numCahceLevels;i++){
        while(!L[i]->ready.read()){
            wait();
        }
        if( L[i]->hit.read()){
          selectHit.write(i);
          selectReady.write(i);
          selectData.write(i);//choose select bit based on cache-hit from L1 to L3, so rdata will be provided  
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
         mem_r.write(true);
         wait();
         mem_r.write(false);
        //wait till main_memory finish to read the data
        while(!mem_ready.read()){
          wait();
        }
        //memory latecny will be waited in main_memory function
        std::vector<uint8_t> cacheLine=main_memory.getCacheLine(addr.read(),cachelineSize);
        for(int i=0;i<numCahceLevels;i++){
            L[i]->write_cacheline(addr.read(),cacheLine);
        }
        for(int i=0;i<MAIN_MEMORY_LATENCY;i++)
            wait();
    }
    if(!dontSetReady){
      std::cout<<"ready in read: "<<L[0]->ready.read()<<", "<<L[1]->ready.read()<<", "<<L[2]->ready.read()<<"\n";
            std::cout<<"hit in read: "<<L[0]->hit.read()<<", "<<L[1]->hit.read()<<", "<<L[2]->hit.read()<<"\n";

         ready.write(true);
    }

    rdata.write(cacheDataOut.read());
  }

  void doWrite() {
    bool hit[numCahceLevels],hitForMiss;
    uint8_t indexL[numCahceLevels];
    for(int i=0;i<numCahceLevels;i++){
        while(!L[i]->ready.read())
            wait();
        if( L[i]->hit.read()){
            hit[i]=true;
            hitForMiss=true;
        }else
            hit[i]=false;
    }
    if(!hitForMiss){
        miss.write(true);
    }
    //the data must be written in rest caches and in main_memory
    mem_w.write(true);
    wait();
    mem_w.write(false);
    while(!mem_ready.read())
      wait();
    for(int i=0;i<numCahceLevels;i++)
        if(!hit[i])
            L[i]->write_cacheline(addr.read(),main_memory.getCacheLine(addr.read(),cachelineSize)); 
    for(int i=0;i<MAIN_MEMORY_LATENCY;i++)
            wait();
    ready.write(true);

    //  ready.write(cacheReadyOut.read());
    //  rdata.write(cacheDataOut.read());
    //  miss.write(cacheHitOut.read());
  }

  

  //returns byte in cache-level: level, cache line: lineIndex, at position: index
  uint8_t getCacheLineContent(uint32_t level, uint32_t lineIndex, uint32_t index){
    if(level<=0 || level>numCahceLevels)
        throw std::runtime_error("Number of Cache Levels must be in range [1;3] in method getCacheLineContent.\n");
    return L[level-1]->getCacheLineContent(lineIndex,index);
  }

  void set_memory(uint32_t address, uint32_t value){
    main_memory.set(address,value);
  }
 
};

#endif // CACHE_HPP


// #ifndef CACHE_HPP
// #define CACHE_HPP

// #include "cache_layer.hpp"
// #include "multiplexer.hpp"
// #include "main_memory.hpp"
// #include <systemc>
// #include <map>
// #include <vector>
// #include <memory>

// #define MAIN_MEMORY_LATENCY 100

// using namespace sc_core;

// SC_MODULE(CACHE) {
//   //inputs
//   sc_in<bool> clk;

//   sc_in<uint32_t> addr, wdata;
//   sc_in<bool> r, w;

//   sc_in<bool> mem_ready;
//   sc_in<uint32_t> mem_rdata;

//   //outputs
//   sc_out<uint32_t> rdata;
//   sc_out<bool> ready,miss;

//   sc_out<uint32_t> mem_addr,mem_wdata;
//   sc_out<bool> mem_r,mem_w;

//   //modules
//   std::vector<std::unique_ptr<CACHE_LAYER>> L;
//   MAIN_MEMORY main_memory;
//   MULTIPLEXER_BOOLEAN cacheHit,cacheReady,rMul,wMul;
//   MULTIPLEXER_I32 cacheData,addrMul,wdataMul;
  
//   //parameters
//   uint8_t numCahceLevels;
//   uint32_t cachelineSize, numLinesL1, numLinesL2, numLinesL3;
//   uint32_t latencyCacheL1,  latencyCacheL2,  latencyCacheL3;
//   uint8_t mappingStrategy;

//   //signals
//   sc_signal<uint32_t> addrMulIn,addrMulOut[3],wdataMulIn,wdataMulOut[3];
//   sc_signal<bool> rMulIn,rMulOut[3],wMulIn,wMulOut[3];

//   sc_signal<uint32_t> addrL[3],wdataL[3],addrMem,wdataMem,rdataMem;
//   sc_signal<bool> rL[3],wL[3],rMem,wMem,readyMem;
//   //multiplexer signals
//   sc_signal<uint8_t> sL[3];
//   sc_signal<uint8_t> selectData,selectHit,selectReady;
//   sc_signal<uint32_t> cacheDataIn[3],cacheDataOut;
//   sc_signal<bool> cacheHitIn[3],cacheHitOut,cacheReadyIn[3],cacheReadyOut;


//   SC_CTOR(CACHE);

//   CACHE(sc_module_name name, uint8_t numCahceLevels, uint32_t cachelineSize,uint32_t numLinesL1,uint32_t numLinesL2,
//     uint32_t numLinesL3, uint32_t latencyCacheL1, uint32_t latencyCacheL2, uint32_t latencyCacheL3, uint8_t mappingStrategy)
//     :sc_module(name),
//     L(numCahceLevels),
//     numCahceLevels(numCahceLevels),
//     cachelineSize(cachelineSize),
//     numLinesL1(numLinesL1),numLinesL2(numLinesL2),numLinesL3(numLinesL3),
//     latencyCacheL1(latencyCacheL1),latencyCacheL2(latencyCacheL2),latencyCacheL3(latencyCacheL3),
//     mappingStrategy(mappingStrategy),
//     cacheData("cacheData",numCahceLevels,1),
//     cacheHit("cacheHit",numCahceLevels,1),
//     cacheReady("cacheReady",numCahceLevels,1),
//     rMul("rMil",1,3),
//     wMul("wMul",1,3),
//     addrMul("addrMul",1,3),
//     wdataMul("wdataMul",1,3),
//     main_memory("main_memory")
//     { 
//       switch (numCahceLevels)
//       {
//         case 3:
//           L[2]=(std::make_unique<CACHE_LAYER>("L3",latencyCacheL3,numLinesL3,cachelineSize,mappingStrategy));
//         case 2:
//           L[1]=(std::make_unique<CACHE_LAYER>("L2",latencyCacheL2,numLinesL2,cachelineSize,mappingStrategy));
//         case 1:
//           L[0]=(std::make_unique<CACHE_LAYER>("L1",latencyCacheL1,numLinesL1,cachelineSize,mappingStrategy));
//           break;
//         default:
//           throw std::runtime_error("Number of Cache Levels must be in range [1;3].\n");
//           break;
//       }

//       addr(addrMulIn); addrMul.in[0](addrMulIn);
//       wdata(wdataMulIn); wdataMul.in[0](wdataMulIn);
//       r(rMulIn); rMul.in[0](rMulIn);
//       w(wMulIn); wMul.in[0](wMulIn);

//       for(int i=0;i<numCahceLevels;i++){
//         L[i]->clk(clk);
//         L[i]->addr(addrMulOut[i]); addrMul.out[i](addrMulOut[i]);
//         L[i]->wdata(wdataMulOut[i]); wdataMul.out[i](wdataMulOut[i]);
//         L[i]->r(rMulOut[i]); rMul.out[i](rMulOut[i]);
//         L[i]->w(wMulOut[i]); wMul.out[i](wMulOut[i]);

//         //L[i]->addr(addrL[i]);addr(addrL[i]);
//         //L[i]->wdata(wdataL[i]);wdata(wdataL[i]);
//         //L[i]->r(rL[i]);r(rL[i]);
//         //L[i]->w(wL[i]);w(wL[i]);
//         L[i]->data(cacheDataIn[i]);cacheData.in[i](cacheDataIn[i]);
//         L[i]->hit(cacheHitIn[i]);cacheHit.in[i](cacheHitIn[i]);
//         L[i]->ready(cacheReadyIn[i]);cacheReady.in[i](cacheReadyIn[i]);

//         sL[i].write(i);
//       }
//       rdata(cacheDataOut); cacheData.out[0](cacheDataOut); cacheData.select(selectData);
//       miss(cacheHit.out[0]); 

//       //miss(cacheHitOut);cacheHit.out[0](cacheHitOut); 
//       cacheHit.select(selectHit);
//       ready(cacheReadyOut);cacheReady.out[0](cacheReadyOut); cacheReady.select(selectReady);
        
//       main_memory.clk(clk);
//       main_memory.addr(addrMem);mem_addr(addrMem);
//       main_memory.wdata(wdataMem);mem_wdata(wdataMem);
//       main_memory.r(rMem); mem_r(rMem);
//       main_memory.w(wMem); mem_w(wMem);
//       main_memory.rdata(rdataMem); mem_rdata(rdataMem);
//       main_memory.ready(readyMem); mem_ready(readyMem);

//     SC_THREAD(behaviour);
//     sensitive << clk.pos();
//   }

//   void behaviour() {
//     while(true) {
//         wait();
//       if (r.read()) {
//         doRead(w.read());
//       }
//       if (w.read()) {
//         doWrite();
//       }
//     }
//   }

//   void doRead(bool dontSetReady) {
//     bool hit=false;
//     uint8_t indexL=0;
//     for(int i=0;i<numCahceLevels;i++){
//         while(!L[i]->ready.read())
//             wait();
//       if( L[i]->hit.read()){
//         cacheData.select(sL[i]);//choose select bit based on cache-hit from L1 to L3, so rdata will be provided  
//         wait(SC_ZERO_TIME);
//         hit=true;
//         indexL=i;
//         break;
//       }
//     }
//     if(hit){
//         for(int i = 0; i <L[indexL]->latency; i++) {
//             wait();//wait letency for cache, where data was found
//         }
//     }else{
//         //cache miss, read data from main_memory and write it to all 3 caches 
//         miss.write(true);
//         //wait till main_memory finish to read the data
//         do{
//             wait();
//         }while(!mem_ready);
//         //memory latecny will be waited in main_memory function
//         std::vector<uint8_t> cacheLine=main_memory.getCacheLine(addr,cachelineSize);
//         for(int i=0;i<numCahceLevels;i++){
//             L[i]->write_cacheline(addr,cacheLine);
//         }
//         for(int i=0;i<MAIN_MEMORY_LATENCY;i++)
//             wait();
//     }
//     if(!dontSetReady)
//         ready.write(true);
//   }

//   void doWrite() {
//     bool hit=false;
//     uint8_t indexL[numCahceLevels];
//     for(int i=0;i<numCahceLevels;i++){
//         while(!L[i]->ready.read())
//             wait();
//         if( L[i]->hit.read()){
//             hit=true;
//             indexL[i]=1;
//             break;
//         }else
//             indexL[i]=0;
//     }
//     if(!hit){
//         miss.write(true);
//     }
//     //the data must be written in rest caches and in main_memory
//     for(int i=0;i<numCahceLevels;i++)
//         if(indexL[i])
//             L[i]->write_cacheline(addr,main_memory.getCacheLine(addr,cachelineSize)); 
//     main_memory.set(addr.read(),rdata.read());
//     ready.write(true);
//   }

  

//   //returns byte in cache-level: level, cache line: lineIndex, at position: index
//   uint8_t getCacheLineContent(uint32_t level, uint32_t lineIndex, uint32_t index){
//     if(level<=0 || level>numCahceLevels)
//         throw std::runtime_error("Number of Cache Levels must be in range [1;3] in method getCacheLineContent.\n");
//     return L[level-1]->getCacheLineContent(lineIndex,index);
//   }

 
// };

// #endif // CACHE_HPP

// #ifndef CACHE_HPP
// #define CACHE_HPP

// #include "cache_layer.hpp"
// #include "multiplexer.hpp"
// #include "main_memory.hpp"
// #include <systemc>
// #include <map>
// #include <vector>
// #include <memory>

// using namespace sc_core;

// SC_MODULE(CACHE) {
//   //inputs
//   sc_in<bool> clk;

//   sc_in<uint32_t> addr, wdata;
//   sc_in<bool> r, w;

//   sc_in<bool> mem_ready;
//   sc_in<uint32_t> mem_rdata;

//   //outputs
//   sc_out<uint32_t> rdata;
//   sc_out<bool> ready,miss;

//   sc_out<uint32_t> mem_addr,mem_wdata;
//   sc_out<bool> mem_r,mem_w;

//   //modules
//   std::vector<std::unique_ptr<CACHE_LAYER>> L;
//   MAIN_MEMORY main_memory;
//   MULTIPLEXER_I32 cacheData;
  
//   //parameters
//   uint8_t numCahceLevels;
//   uint32_t cachelineSize, numLinesL1, numLinesL2, numLinesL3;
//   uint32_t latencyCacheL1,  latencyCacheL2,  latencyCacheL3;
//   uint8_t mappingStrategy;

//   //signals
//   sc_signal<uint32_t> addrL[3],wdataL[3],addrMem,wdataMem;
//   sc_signal<bool> rL[3],wL[3],rMem,wMem;
//   sc_signal<uint8_t> sL[3];

//   SC_CTOR(CACHE);

//   CACHE(sc_module_name name, uint8_t numCahceLevels, uint32_t cachelineSize,uint32_t numLinesL1,uint32_t numLinesL2,
//     uint32_t numLinesL3, uint32_t latencyCacheL1, uint32_t latencyCacheL2, uint32_t latencyCacheL3, uint8_t mappingStrategy)
//     :sc_module(name),
//     L(numCahceLevels),
//     numCahceLevels(numCahceLevels),
//     cachelineSize(cachelineSize),
//     numLinesL1(numLinesL1),numLinesL2(numLinesL2),numLinesL3(numLinesL3),
//     latencyCacheL1(latencyCacheL1),latencyCacheL2(latencyCacheL2),latencyCacheL3(latencyCacheL3),
//     mappingStrategy(mappingStrategy),
//     cacheData("cacheData",numCahceLevels,1),
//     main_memory("main_memory")
//     { 
//       switch (numCahceLevels)
//       {
//         case 3:
//           L[2]=(std::make_unique<CACHE_LAYER>("L3",latencyCacheL3,numLinesL3,cachelineSize,mappingStrategy));
//         case 2:
//           L[1]=(std::make_unique<CACHE_LAYER>("L2",latencyCacheL2,numLinesL2,cachelineSize,mappingStrategy));
//         case 1:
//           L[0]=(std::make_unique<CACHE_LAYER>("L1",latencyCacheL1,numLinesL1,cachelineSize,mappingStrategy));
//           break;
//         default:
//           throw std::runtime_error("Number of Cache Levels must be in range [1;3].\n");
//           break;
//       }

//       for(int i=0;i<numCahceLevels;i++){
//         L[i]->clk(clk);
//         L[i]->addr(addrL[i]);
//         addr(addrL[i]);
//         L[i]->wdata(wdataL[i]);
//         wdata(wdataL[i]);
//         L[i]->r(rL[i]);
//         r(rL[i]);
//         L[i]->w(wL[i]);
//         w(wL[i]);
//         cacheData.in[i](L[i]->data);
//         rdata(cacheData.out[0]);
//         sL[i].write(i);
//       }
      
//       main_memory.clk(clk);
//       main_memory.addr(addrMem);
//       addr(addrMem);
//       main_memory.wdata(wdataMem);
//       wdata(wdataMem);
//       main_memory.r(rMem);
//       main_memory.w(wMem);

//     SC_THREAD(behaviour);
//     sensitive << clk.pos();
//   }

//   void behaviour() {
//     while(true) {
//         wait();
//       if (r.read()) {
//         doRead(w.read());
//       }
//       if (w.read()) {
//         doWrite();
//       }
//     }
//   }

//   void doRead(bool dontSetReady) {
//     bool hit=false;
//     uint8_t indexL=0;
//     for(int i=0;i<numCahceLevels;i++){
//         while(!L[i]->ready.read())
//             wait();
//       if( L[i]->hit.read()){
//         cacheData.select(sL[i]);//choose select bit based on cache-hit from L1 to L3, so rdata will be provided  
//         wait(SC_ZERO_TIME);
//         hit=true;
//         indexL=i;
//         break;
//       }
//     }
//     if(hit){
//         for(int i = 0; i <L[indexL]->latency; i++) {
//             wait();//wait letency for cache, where data was found
//         }
//     }else{
//         //cache miss, read data from main_memory and write it to all 3 caches 
//         miss.write(true);
//         //wait till main_memory finish to read the data
//         do{
//             wait();
//         }while(!mem_ready);
//         //memory latecny will be waited in main_memory function
//         for(int i=0;i<numCahceLevels;i++){
//             L[i]->write_data_from_main_memory(NULL,mem_rdata.read());//i dont think there should be argument for addres, bc you dont need adress for fifo or lifo
//         }
//     }
//     if(!dontSetReady)
//         ready.write(true);
//   }

//   void doWrite() {
//     bool hit=false;
//     uint8_t indexL=0;
//     for(int i=0;i<numCahceLevels;i++){
//         while(!L[i]->ready.read())
//             wait();
//         if( L[i]->hit.read()){
//             hit=true;
//             indexL=i;
//             break;
//       }
//     }
//     if(!hit){
//         miss.write(true);
//         indexL=255;
//     }
//     //the data must be written in rest caches and in main_memory
//     for(int i=0;i<numCahceLevels;i++)
//         if(i!=indexL)
//             L[i]->write_data_from_main_memory(NULL,rdata.read());//rdata was set in cache_layer 
//     main_memory.set(addr.read(),rdata.read());
//     ready.write(true);
//   }

  

//   //returns byte in cache-level: level, cache line: lineIndex, at position: index
//   uint8_t getCacheLineContent(uint32_t level, uint32_t lineIndex, uint32_t index){
//     if(level<=0 || level>numCahceLevels)
//         throw std::runtime_error("Number of Cache Levels must be in range [1;3] in method getCacheLineContent.\n");
//     return L[level-1]->getCacheLineContent(lineIndex,index);
//   }

 
// };

// #endif // CACHE_HPP