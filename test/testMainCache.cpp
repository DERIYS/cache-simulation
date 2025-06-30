#include <iostream>
#include <functional>
#include <string>
#include "cache.hpp"


// Helper function to compare integers
void assert_equal(const std::string &test_name, uint32_t expected, uint32_t actual) {
    if (expected == actual) {
        std::cout << test_name << ": PASSED\n";
    } else {
        std::cout << test_name << ": FAILED (Expected " << std::hex << expected
                  << ", got " << actual << ")\n";
    }
}

// Helper function to compare booleans
void assert_bool(const std::string& test_name, const bool expected, const bool actual) {
    if (expected == actual) {
        std::cout << test_name << ": PASSED\n";
    } else {
        std::cout << test_name << ": FAILED (Expected " << (expected ? "true" : "false")
                  << ", got " << (actual ? "true" : "false") << ")\n";
    }
}

// Helper function for exception handling (doesn't work since systemc modules have their own threads on which they throw the exceptions)
void assert_throws(const std::string& test_name, const std::function<void()>& func) {
    try {
        func();
        std::cout << test_name << ": FAILED (Expected exception, none thrown)\n";
    } catch (const std::runtime_error& e) {
        std::cout << test_name << ": PASSED\n";
    }
}

void cacheReadMiss(CACHE &cache, sc_signal<uint32_t> &addr,sc_signal<uint32_t> &rdata,  sc_signal<bool> &r, 
    sc_signal<bool> &w, sc_signal<bool> &miss, sc_signal<bool> &ready){
        addr.write(0);
        w.write(false);
        r.write(true);
        sc_start(10,SC_NS);
        r.write(false);
        sc_start(140,SC_NS);//2 cycle wait cache,1 c. wait memory,10 c. latency memory,+2 c.
          
        //sc_start(165, SC_NS);
        assert_equal("cacheReadMiss rdata",0xA1A2A3A4,rdata.read());
        assert_bool("cacheReadMiss miss",true,miss.read());
        assert_bool("cacheReadMiss ready",true,ready.read());
    
        assert_equal("cacheReadMiss L1[0]",cache.getCacheLineContent(1,0,0),0xa4);
        assert_equal("cacheReadMiss L1[1]",cache.getCacheLineContent(1,0,1),0xa3);
        assert_equal("cacheReadMiss L1[2]",cache.getCacheLineContent(1,0,2),0xa2);
        assert_equal("cacheReadMiss L1[3]",cache.getCacheLineContent(1,0,3),0xa1);

        assert_equal("cacheReadMiss L2[0]",cache.getCacheLineContent(2,0,0),0xa4);
        assert_equal("cacheReadMiss L2[1]",cache.getCacheLineContent(2,0,1),0xa3);
        assert_equal("cacheReadMiss L2[2]",cache.getCacheLineContent(2,0,2),0xa2);
        assert_equal("cacheReadMiss L2[3]",cache.getCacheLineContent(2,0,3),0xa1);

        assert_equal("cacheReadMiss L3[0]",cache.getCacheLineContent(3,0,0),0xa4);
        assert_equal("cacheReadMiss L3[1]",cache.getCacheLineContent(3,0,1),0xa3);
        assert_equal("cacheReadMiss L3[2]",cache.getCacheLineContent(3,0,2),0xa2);
        assert_equal("cacheReadMiss L3[3]",cache.getCacheLineContent(3,0,3),0xa1);
}

void cacheReadHit(CACHE &cache, sc_signal<uint32_t> &addr,sc_signal<uint32_t> &rdata,  sc_signal<bool> &r, 
    sc_signal<bool> &w, sc_signal<bool> &miss, sc_signal<bool> &ready){

        addr.write(0);
        w.write(false);
        r.write(true);
        sc_start(10,SC_NS);
        r.write(false);
        sc_start(20, SC_NS);//l1 latency =1

        assert_equal("cacheReadHit",0xA1A2A3A4,rdata.read());
        assert_bool("cacheReadHit",false,miss.read());
        assert_bool("cacheReadHit",true,ready.read());
}

void cacheReadHitB(CACHE &cache, sc_signal<uint32_t> &addr,sc_signal<uint32_t> &rdata,  sc_signal<bool> &r, 
    sc_signal<bool> &w, sc_signal<bool> &miss, sc_signal<bool> &ready){

        addr.write(0x4);
        w.write(false);
        r.write(true);
        sc_start(10,SC_NS);
        r.write(false);
        sc_start(40, SC_NS);//l2 latency =2

        assert_equal("cacheReadHitB",0xb1b2b3b4,rdata.read());
        assert_bool("cacheReadHitB",false,miss.read());
        assert_bool("cacheReadHitB",true,ready.read());
}

void cacheReadSpatialLocalityHit(CACHE &cache, sc_signal<uint32_t> &addr,sc_signal<uint32_t> &rdata,  sc_signal<bool> &r, 
    sc_signal<bool> &w, sc_signal<bool> &miss, sc_signal<bool> &ready){

        addr.write(0x4);
        w.write(false);
        r.write(true);
        sc_start(10,SC_NS);
        r.write(false);
        sc_start(20, SC_NS);//l1 latency =1

        assert_equal("cacheReadSpatialLocalityHit",0xB1B2B3B4,rdata.read());
        assert_bool("cacheReadSpatialLocalityHit",false  ,miss.read());
        assert_bool("cacheReadSpatialLocalityHit",true,ready.read());
}

void cacheWriteHit(CACHE &cache, sc_signal<uint32_t> &addr,sc_signal<uint32_t> &wdata,  sc_signal<bool> &r, 
    sc_signal<bool> &w, sc_signal<bool> &miss, sc_signal<bool> &ready){
        addr.write(0);
        w.write(true);
        r.write(false);
        wdata.write(0xd1d2d3d4);
        sc_start(10,SC_NS);
        w.write(false);
        sc_start(100,SC_NS);//2 cycle wait cache,1 c. wait memory,10 c. latency memory,+2 c.
        
        assert_bool("cacheWriteHit",false  ,miss.read());
        assert_bool("cacheWriteHit",true,ready.read());

        cache.L[0]->print(1);
        cache.L[1]->print(2);
        cache.L[2]->print(3);

         assert_equal("cacheReadMiss L1[0]",cache.getCacheLineContent(1,0,0),0xd4);
        assert_equal("cacheReadMiss L1[1]",cache.getCacheLineContent(1,0,1),0xd3);
        assert_equal("cacheReadMiss L1[2]",cache.getCacheLineContent(1,0,2),0xd2);
        assert_equal("cacheReadMiss L1[3]",cache.getCacheLineContent(1,0,3),0xd1);
}

void cacheWriteMiss(CACHE &cache, sc_signal<uint32_t> &addr,sc_signal<uint32_t> &wdata,  sc_signal<bool> &r, 
    sc_signal<bool> &w, sc_signal<bool> &miss, sc_signal<bool> &ready){
        addr.write(8);
        w.write(true);
        r.write(false);
        wdata.write(0xe1e2e3e4);
        sc_start(10,SC_NS);
        w.write(false);
        sc_start(140,SC_NS);//2 cycle wait cache,1 c. wait memory,10 c. latency memory,+2 c.
        assert_bool("cacheWriteHit",true  ,miss.read());
        assert_bool("cacheWriteHit",true,ready.read());

        cache.L[0]->print(1);
        cache.L[1]->print(2);
        cache.L[2]->print(3);

         assert_equal("cacheReadMiss L1[0]",cache.getCacheLineContent(1,0,0),0xe4);
        assert_equal("cacheReadMiss L1[1]",cache.getCacheLineContent(1,0,1),0xe3);
        assert_equal("cacheReadMiss L1[2]",cache.getCacheLineContent(1,0,2),0xe2);
        assert_equal("cacheReadMiss L1[3]",cache.getCacheLineContent(1,0,3),0xe1);
}

int sc_main(int argc, char *argv[])
{
    sc_clock clk("clk", 10, SC_NS);
    sc_signal<uint32_t> addr, wdata;
    sc_signal<bool> r, w, miss, ready;
    sc_signal<uint32_t> rdata;
    //name,numCahceLevels,cachelineSize,numLinesL1,2,3,latencyCacheL1,2,3, mappingStrategy
 
    CACHE cache("cache",3,8,1,2,2,2,2,3,0);
    cache.test=true;
    //CACHE_LAYER cache("cache", 1, 4, 16, 0); // Default: direct-mapped
    cache.clk(clk);
    cache.addr(addr);
    cache.wdata(wdata);
    cache.r(r);
    cache.w(w);
    cache.miss(miss);
    cache.ready(ready);////
    cache.rdata(rdata);

    cache.set_memory(0,0xA1A2A3A4);
    cache.set_memory(4,0xB1B2B3B4);
    cache.set_memory(8,0xC1C2C3C4);

    cacheReadMiss(cache,addr,rdata,r,w,miss,ready);
    cacheReadHit(cache,addr,rdata,r,w,miss,ready);
    cacheReadSpatialLocalityHit(cache,addr,rdata,r,w,miss,ready);

    cacheWriteHit(cache,addr,wdata,r,w,miss,ready);
    cacheWriteMiss(cache,addr,wdata,r,w,miss,ready);
    
    cacheReadHitB(cache,addr,rdata,r,w,miss,ready);


    
    return 0;

}