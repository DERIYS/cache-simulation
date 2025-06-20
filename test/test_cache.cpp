#include <functional>
#include "../include/cache_layer.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>

// to compile:
// -> export LD_LIBRARY_PATH=$(SYSTEMC_HOME)/lib:$LD_LIBRARY_PATH
// -> g++ -std=c++14 -I$(SYSTEMC_HOME)/include -L$(SYSTEMC_HOME)/lib test_cache.cpp -lsystemc -o test_cache
// where $(SYSTEMC_HOME) is your path to systemc

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

void test_direct_mapped_read_hit(CACHE_LAYER& cache, sc_signal<uint32_t>& addr, sc_signal<bool>& r, sc_signal<bool>& w,
                           const sc_signal<bool>& hit, const sc_signal<bool>& ready, const sc_signal<uint32_t>& data) {
    // Address: 0x00010008 -> Tag=0x00010, Index=0, Offset=0x8 | Offset-Bits = 4, Index-Bits = 2
    std::vector<CacheLine> memory(4, {0, false, std::vector<uint8_t>(16)});
    memory[0].tag = 0x00010008 >> 6;
    memory[0].valid = true;
    // Data: 0x12345678 (Little-Endian) with Offset 8
    memory[0].data[8] = 0x78;  // LSB
    memory[0].data[9] = 0x56;
    memory[0].data[10] = 0x34;
    memory[0].data[11] = 0x12; // MSB

    cache.set_memory(memory, {}, {});
    addr.write(0x00010008);
    r.write(true);
    w.write(false);
    sc_start(10, SC_NS); // One clock cycle
    r.write(false);

    assert_bool("DirectMappedHit_Hit", true, hit.read());
    assert_equal("DirectMappedHit_Data", 0x12345678, data.read());
    assert_bool("DirectMappedHit_Ready", true, ready.read());
}

void test_direct_mapped_read_miss(CACHE_LAYER& cache, sc_signal<uint32_t>& addr, sc_signal<bool>& r, sc_signal<bool>& w,
                            const sc_signal<bool>& hit, const sc_signal<bool>& ready, sc_signal<uint32_t>& data) {
    // Address: 0x00010008, false Tag
    std::vector<CacheLine> memory(4, {0, false, std::vector<uint8_t>(16)});
    memory[0].tag = 0x00020;
    memory[0].valid = true;

    cache.set_memory(memory, {}, {});
    addr.write(0x00010008);
    r.write(true);
    w.write(false);
    sc_start(10, SC_NS);
    r.write(false);

    assert_bool("DirectMappedMiss_Hit", false, hit.read());
    assert_bool("DirectMappedMiss_Ready", true, ready.read());
}

void test_direct_mapped_invalid_offset(CACHE_LAYER& cache, sc_signal<uint32_t>& addr, sc_signal<bool>& r, sc_signal<bool>& w) {
    // Address: 0x0001000E, Offset=0xE
    std::vector<CacheLine> memory(4, {0, false, std::vector<uint8_t>(16)});
    cache.set_memory(memory, {}, {});
    addr.write(0x0001000E);
    r.write(true);
    w.write(false);

    assert_throws("DirectMappedInvalidOffset", [&]() { sc_start(10, SC_NS); });
}

void test_fully_associative_read_hit(CACHE_LAYER& cache, sc_signal<uint32_t>& addr, sc_signal<bool>& r, sc_signal<bool>& w,
                               const sc_signal<bool>& hit, const sc_signal<bool>& ready, const sc_signal<uint32_t>& data) {
    cache.mapping_strategy = 1; // Fully associative
    // Address: 0x10000008 -> Tag=0x1000000, Offset=0x8
    std::vector<CacheLine> memory(4, {0, false, std::vector<uint8_t>(16)});
    memory[1].tag = 0x1000000;
    memory[1].valid = true;
    // Data: 0x87654321 (Little-Endian) with Offset 8
    memory[1].data[8] = 0x21;  // LSB
    memory[1].data[9] = 0x43;
    memory[1].data[10] = 0x65;
    memory[1].data[11] = 0x87; // MSB

    std::list<uint32_t> lru = {1};
    std::unordered_map<uint32_t, std::list<uint32_t>::iterator> lru_map;
    lru_map[0x1000000] = lru.begin();

    cache.set_memory(memory, lru, lru_map);
    addr.write(0x10000008);
    r.write(true);
    w.write(false);
    sc_start(10, SC_NS);
    r.write(false);

    assert_bool("FullyAssociativeHit_Hit", true, hit.read());
    assert_equal("FullyAssociativeHit_Data", 0x87654321, data.read());
    assert_bool("FullyAssociativeHit_Ready", true, ready.read());
    // Überprüfe LRU-Update
    assert_equal("FullyAssociativeHit_LRU", 1, cache.lru_list.front());
}

// Test-Funktion für Fully Associative Cache-Miss
void test_fully_associative_read_miss(CACHE_LAYER& cache, sc_signal<uint32_t>& addr, sc_signal<bool>& r, sc_signal<bool>& w,
                                const sc_signal<bool>& hit, const sc_signal<bool>& ready) {
    cache.mapping_strategy = 1; // Fully associative
    std::vector<CacheLine> memory(4, {0, false, std::vector<uint8_t>(16)});
    std::list<uint32_t> lru;
    std::unordered_map<uint32_t, std::list<uint32_t>::iterator> lru_map;

    cache.set_memory(memory, lru, lru_map);
    addr.write(0x10000008);
    r.write(true);
    w.write(false);
    sc_start(10, SC_NS);
    r.write(false);

    assert_bool("FullyAssociativeMiss_Hit", false, hit.read());
    assert_bool("FullyAssociativeMiss_Ready", true, ready.read());
}

void test_fully_associative_invalid_offset(CACHE_LAYER& cache, sc_signal<uint32_t>& addr, sc_signal<bool>& r, sc_signal<bool>& w) {
    cache.mapping_strategy = 1; // Fully associative
    std::vector<CacheLine> memory(4, {0, false, std::vector<uint8_t>(16)});
    cache.set_memory(memory, {}, {});
    addr.write(0x1000000E);
    r.write(true);
    w.write(false);

    assert_throws("FullyAssociativeInvalidOffset", [&]() { sc_start(10, SC_NS); });
}

int sc_main(int argc, char* argv[]) {
    sc_clock clk("clk", 10, SC_NS);
    sc_signal<uint32_t> addr, wdata;
    sc_signal<bool> r, w, hit, ready;
    sc_signal<uint32_t> data;

    CACHE_LAYER cache("cache", 1, 4, 16, 0); // Default: direct-mapped
    cache.clk(clk);
    cache.addr(addr);
    cache.wdata(wdata);
    cache.r(r);
    cache.w(w);
    cache.hit(hit);
    cache.ready(ready);
    cache.data(data);

    std::cout << "Running Direct-Mapped Tests...\n";
    test_direct_mapped_read_hit(cache, addr, r, w, hit, ready, data);
    test_direct_mapped_read_miss(cache, addr, r, w, hit, ready, data);
    // test_direct_mapped_invalid_offset(cache, addr, r, w);

    std::cout << "\nRunning Fully Associative Tests...\n";
    test_fully_associative_read_hit(cache, addr, r, w, hit, ready, data);
    test_fully_associative_read_miss(cache, addr, r, w, hit, ready);
    // test_fully_associative_invalid_offset(cache, addr, r, w);
    return 0;
}