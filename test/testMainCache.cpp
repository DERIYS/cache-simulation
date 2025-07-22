#include <iostream>
#include <functional>
#include <string>
#include "../include/cache.hpp"


// Helper function to compare integers
void assert_equal(const std::string &test_name, uint32_t expected, uint32_t actual) {
    if (expected == actual) {
        std::cout << test_name << ": PASSED\n";
    } else {
        std::cout << test_name << ": FAILED (Expected " << std::hex << expected
                  << ", got " << actual << ")\n";
    }
}

void assert_equal_one_line(const std::string &test_name, uint32_t expected, uint32_t actual) {
    if (expected == actual) {
        std::cout << test_name << ": PASSED ";
    } else {
        std::cout << test_name << ": FAILED (Expected " << std::hex << expected
                  << ", got " << actual << ") ";
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
    sc_signal<bool> &w, sc_signal<bool> &miss, sc_signal<bool> &ready,uint32_t rdata_expected){
        w.write(false);
        r.write(true);
        sc_start(120,SC_NS);
        r.write(false);
        // sc_start(140,SC_NS);//2 cycle wait cache,1 c. wait memory,10 c. latency memory,+2 c.
          
        //sc_start(165, SC_NS);
        assert_equal("cacheReadMiss rdata",rdata_expected,rdata.read());
        assert_bool("cacheReadMiss miss",true,miss.read());
        assert_bool("cacheReadMiss ready",true,ready.read());
}

void cacheReadHit(CACHE &cache, sc_signal<uint32_t> &addr,sc_signal<uint32_t> &rdata,  sc_signal<bool> &r, 
    sc_signal<bool> &w, sc_signal<bool> &miss, sc_signal<bool> &ready,uint32_t rdata_expected){

        // addr.write(0);
        w.write(false);
        r.write(true);
        //  sc_start(20,SC_NS);
        //  r.write(false);
        sc_start(100, SC_NS);//l1 latency =1
        r.write(false);
        assert_equal("cacheReadHit rdata",rdata_expected,rdata.read());
        assert_bool("cacheReadHit miss",false,miss.read());
        assert_bool("cacheReadHit ready",true,ready.read());
}


void cacheReadSpatialLocalityHit(CACHE &cache, sc_signal<uint32_t> &addr,sc_signal<uint32_t> &rdata,  sc_signal<bool> &r, 
    sc_signal<bool> &w, sc_signal<bool> &miss, sc_signal<bool> &ready){
        addr.write(0x4);
        w.write(false);
        r.write(true);
        // sc_start(10,SC_NS);
        // r.write(false);
        sc_start(70, SC_NS);//l1 latency =1

        assert_equal("cacheReadSpatialLocalityHit rdata",0xB1B2B3B4,rdata.read());
        assert_bool("cacheReadSpatialLocalityHit miss",false  ,miss.read());
        assert_bool("cacheReadSpatialLocalityHit ready",true,ready.read());
}

void cacheWriteHit(CACHE &cache, sc_signal<uint32_t> &addr,sc_signal<uint32_t> &wdata,  sc_signal<bool> &r, 
    sc_signal<bool> &w, sc_signal<bool> &miss, sc_signal<bool> &ready){
        // addr.write(0);
        w.write(true);
        r.write(false);
        // wdata.write(0xd1d2d3d4);
        // sc_start(10,SC_NS);
        // w.write(false);
        sc_start(100,SC_NS);//2 cycle wait cache,1 c. wait memory,10 c. latency memory,+2 c.
        w.write(false);
        assert_bool("cacheWriteHit miss",false  ,miss.read());
        assert_bool("cacheWriteHit ready",true,ready.read());

        assert_equal_one_line("cacheReadAfterWriteHit L1[0]",0xd4,cache.get_cache_line_content(1,0,0));
        assert_equal_one_line("cacheReadAfterWriteHit L1[1]",0xd3,cache.get_cache_line_content(1,0,1));
        assert_equal_one_line("cacheReadAfterWriteHit L1[2]",0xd2,cache.get_cache_line_content(1,0,2));
        assert_equal("cacheReadAfterWriteHit L1[3]",0xd1,cache.get_cache_line_content(1,0,3));
}

void cacheWriteMiss(CACHE &cache, sc_signal<uint32_t> &addr,sc_signal<uint32_t> &wdata,  sc_signal<bool> &r, 
    sc_signal<bool> &w, sc_signal<bool> &miss, sc_signal<bool> &ready){
        // addr.write(0x8);
        w.write(true);
        r.write(false);
        // wdata.write(0xe1e2e3e4);
        sc_start(140,SC_NS);
        w.write(false);
        // sc_start(140,SC_NS);//2 cycle wait cache,1 c. wait memory,10 c. latency memory,+2 c.
        assert_bool("cacheWriteMiss miss",true  ,miss.read());
        assert_bool("cacheWriteMiss ready ",true,ready.read());
}

void print_caches(int number,CACHE& cache){
    for(int i=0;i<number;i++){
        cache.L[i]->print_internal_memory(i+1);
    }std::cout<<"\n";
}

int sc_main(int argc, char *argv[])
{
    sc_clock clk("clk", 10, SC_NS);
    sc_signal<uint32_t> addr, wdata;
    sc_signal<bool> r, w, miss, ready;
    sc_signal<uint32_t> rdata;
    //name,numCahceLevels,cachelineSize,numLinesL1,2,3,latencyCacheL1,2,3, mappingStrategy
 
    CACHE cache("cache",3,8,2,4,8,2,2,3,0);

    // cache.L[0]->test_mode=true;cache.L[1]->test_mode=true;cache.L[2]->test_mode=true;
    // cache.test=true;
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
    cache.set_memory(0xC,0x12345678);
    cache.set_memory(0x10, 0xabababab);
    cache.set_memory(0x14,0xabcdefab);
    cache.set_memory(0x18,0xa1aab1bb);
    cache.set_memory(0x1C,0x11111111);
    cache.set_memory(0x20, 0x123c123f);
    cache.set_memory(0x24,0xffffffff);
    cache.set_memory(0x28,0xf1c42071);
    cache.set_memory(0x2C,0x22222222);
    cache.set_memory(0x30, 0xaefaefea);
    cache.set_memory(0x34,0xedaedaff);
    cache.set_memory(0x38,0x45454545);
    cache.set_memory(0x3C,0xda913da);
    cache.set_memory(0x40, 0x838995);
    cache.set_memory(0x44,0x838547);
    cache.set_memory(0x48,0x7);

    std::cout<<"main_memory init:\n";
    cache.main_memory.print();
    std::cout<<"\n";

    //read addr=0, write a cacheline to each cache-level
    std::cout<<"read 0x0, cache miss, write the whole cacheline in each cache level: \n";
    addr.write(0);
    cacheReadMiss(cache,addr,rdata,r,w,miss,ready,0xa1a2a3a4);
    print_caches(3,cache);
    
    //read addr=0 from cache L1
    std::cout<<"read 0x0, cache hit in L1: \n";
    addr.write(0);
    cacheReadHit(cache,addr,rdata,r,w,miss,ready,0xa1a2a3a4);

    //read addr=4 
    std::cout<<"\nread 0x4, cache hit in L1 bc of spatial locality: \n";
    cacheReadSpatialLocalityHit(cache,addr,rdata,r,w,miss,ready);

    //write addr=0 0xd1d2d3d4 to main-memory and each cache-level
    std::cout<<"\nwrite 0xd1d2d3d4 in 0x0, cache hit, no need to rewrite the whole cacheline: \n";
    addr.write(0); wdata.write(0xd1d2d3d4);
    cacheWriteHit(cache,addr,wdata,r,w,miss,ready);
    print_caches(3,cache);

    //write addr=8 0xe1e2e3e4 to mm and caches
    std::cout<<"write 0xe1e2e3e4 in 0x8, cache miss, whole cacheline: \n";
    addr.write(8);wdata.write(0xe1e2e3e4);
    cacheWriteMiss(cache,addr,wdata,r,w,miss,ready);
    print_caches(3,cache);
    cache.main_memory.print();


    std::cout<<"fill all caches in order using readMiss,check fifo princip in all caches\n";
    addr.write(0xC);
    cacheReadHit(cache,addr,rdata,r,w,miss,ready,0x12345678);

    addr.write(0x14);
    cacheReadMiss(cache,addr,rdata,r,w,miss,ready,0xabcdefab);

    addr.write(0x1c);
    cacheReadMiss(cache,addr,rdata,r,w,miss,ready,0x11111111);

    addr.write(0x24);
    cacheReadMiss(cache,addr,rdata,r,w,miss,ready,0xffffffff);

    addr.write(0x2c);
    cacheReadMiss(cache,addr,rdata,r,w,miss,ready,0x22222222);

    addr.write(0x34);
    cacheReadMiss(cache,addr,rdata,r,w,miss,ready,0xedaedaff);

    addr.write(0x3c);
    cacheReadMiss(cache,addr,rdata,r,w,miss,ready,0xda913da);

    addr.write(0x44);
    cacheReadMiss(cache,addr,rdata,r,w,miss,ready,0x838547);

    print_caches(3,cache);

    std::cout<<"check cache hit:\n";
    addr.write(0xC);
    cacheReadHit(cache,addr,rdata,r,w,miss,ready,0x12345678);

    addr.write(0x10);
    cacheReadHit(cache,addr,rdata,r,w,miss,ready,0xabababab);

    addr.write(0x18);
    cacheReadHit(cache,addr,rdata,r,w,miss,ready,0xa1aab1bb);

    addr.write(0x20);
    cacheReadHit(cache,addr,rdata,r,w,miss,ready,0x123c123f);

    addr.write(0x28);
    cacheReadHit(cache,addr,rdata,r,w,miss,ready,0xf1c42071);

    addr.write(0x2c);
    cacheReadHit(cache,addr,rdata,r,w,miss,ready,0x22222222);

    addr.write(0x34);
    cacheReadHit(cache,addr,rdata,r,w,miss,ready,0xedaedaff);

    addr.write(0x3c);
    cacheReadHit(cache,addr,rdata,r,w,miss,ready,0xda913da);

    addr.write(0x44);
    cacheReadHit(cache,addr,rdata,r,w,miss,ready,0x838547);

    return 0;

}