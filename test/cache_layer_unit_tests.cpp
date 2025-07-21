#include <functional>
#include "../include/cache_layer.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <iomanip>

// to compile:
// -> export LD_LIBRARY_PATH=$(SYSTEMC_HOME)/lib:$LD_LIBRARY_PATH
// -> g++ -std=c++14 -I$(SYSTEMC_HOME)/include -L$(SYSTEMC_HOME)/lib test_cache.cpp -lsystemc -o test_cache
// where $(SYSTEMC_HOME) is your path to systemc

// Helper function to compare integers

void assert_equal_layer(const std::string &test_name, uint32_t expected, uint32_t actual)
{
    if (expected == actual)
    {
        std::cout << test_name << ": PASSED\n";
    }
    else
    {
        std::cout << test_name << ": FAILED (Expected " << std::hex << expected
                  << ", got " << actual << ")\n";
    }
}

// Helper function to compare booleans
void assert_bool_layer(const std::string &test_name, const bool expected, const bool actual)
{
    if (expected == actual)
    {
        std::cout << test_name << ": PASSED\n";
    }
    else
    {
        std::cout << test_name << ": FAILED (Expected " << (expected ? "true" : "false")
                  << ", got " << (actual ? "true" : "false") << ")\n";
    }
}

// Helper function for exception handling (doesn't work since systemc modules have their own threads on which they throw the exceptions)
void assert_throws_layer(const std::string &test_name, const std::function<void()> &func)
{
    try
    {
        func();
        std::cout << test_name << ": FAILED (Expected exception, none thrown)\n";
    }
    catch (const std::runtime_error &e)
    {
        std::cout << test_name << ": PASSED\n";
    }
}

// Helper function to compare data in cache_layer lines
void assert_data(const std::string &test_name, const std::vector<uint8_t> &expected, const std::vector<uint8_t> &actual)
{
    if (expected == actual)
    {
        std::cout << test_name << ": PASSED\n";
    }
    else
    {
        std::cout << test_name << ": FAILED (Expected data mismatch)\n\tExpected: ";
        for (const uint8_t byte : expected)
        {
            std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)byte << " ";
        }
        std::cout << std::flush;

        std::cout << "\n\tActual:   ";
        for (const uint8_t byte : actual)
        {
            std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)byte << " ";
        }
        std::cout << "\n"
                  << std::flush;
    }
}

void test_direct_mapped_read_hit(CACHE_LAYER &cache_layer, sc_signal<uint32_t> &addr, sc_signal<bool> &r, sc_signal<bool> &w,
                                 const sc_signal<bool> &miss, const sc_signal<bool> &ready, const sc_signal<uint32_t> &data)
{
    // Address: 0x00010008 -> Tag=0x00010, Index=0, Offset=0x8 | Offset-Bits = 4, Index-Bits = 2
    std::vector<CacheLine> memory(4, {0, false, std::vector<uint8_t>(16)});
    memory[0].tag = 0x00010008 >> 6;
    memory[0].valid = true;
    // Data: 0x12345678 (Little-Endian) with Offset 8
    memory[0].data[8] = 0x78; // LSB
    memory[0].data[9] = 0x56;
    memory[0].data[10] = 0x34;
    memory[0].data[11] = 0x12; // MSB

    cache_layer.set_memory(memory, {}, {});
    addr.write(0x00010008);
    r.write(true);
    w.write(false);
    sc_start(10, SC_NS); // One clock cycle
    r.write(false);

    assert_bool_layer("DirectMappedHit_Miss", false, miss.read());
    assert_equal_layer("DirectMappedHit_Data", 0x12345678, data.read());
    assert_bool_layer("DirectMappedHit_Ready", true, ready.read());
}

void test_direct_mapped_read_miss(CACHE_LAYER &cache_layer, sc_signal<uint32_t> &addr, sc_signal<bool> &r, sc_signal<bool> &w,
                                  const sc_signal<bool> &miss, const sc_signal<bool> &ready, sc_signal<uint32_t> &data)
{
    // Address: 0x00010008, false Tag
    std::vector<CacheLine> memory(4, {0, false, std::vector<uint8_t>(16)});
    memory[0].tag = 0x00020;
    memory[0].valid = true;

    cache_layer.set_memory(memory, {}, {});
    addr.write(0x00010008);
    r.write(true);
    w.write(false);
    sc_start(10, SC_NS);
    r.write(false);

    assert_bool_layer("DirectMappedMiss_Miss", true, miss.read());
    assert_bool_layer("DirectMappedMiss_Ready", true, ready.read());
}

void test_direct_mapped_invalid_offset(CACHE_LAYER &cache_layer, sc_signal<uint32_t> &addr, sc_signal<bool> &r, sc_signal<bool> &w)
{
    // Address: 0x0001000E, Offset=0xE
    std::vector<CacheLine> memory(4, {0, false, std::vector<uint8_t>(16)});
    cache_layer.set_memory(memory, {}, {});
    addr.write(0x0001000E);
    r.write(true);
    w.write(false);
    cout << "Testing DirectMappedInvalidOffset" << endl;
    sc_start(10, SC_NS);
    cout << "Testing DirectMappedInvalidOffset" << endl;

    assert_bool_layer("DirectMappedInvalidOffset", true, cache_layer.error);
}

void test_fully_associative_read_hit(CACHE_LAYER &cache_layer, sc_signal<uint32_t> &addr, sc_signal<bool> &r, sc_signal<bool> &w,
                                     const sc_signal<bool> &miss, const sc_signal<bool> &ready, const sc_signal<uint32_t> &data)
{
    cache_layer.mapping_strategy = 1; // Fully associative
    // Address: 0x10000008 -> Tag=0x1000000, Offset=0x8
    std::vector<CacheLine> memory(4, {0, false, std::vector<uint8_t>(16)});
    memory[1].tag = 0x1000000;
    memory[1].valid = true;
    // Data: 0x87654321 (Little-Endian) with Offset 8
    memory[1].data[8] = 0x21; // LSB
    memory[1].data[9] = 0x43;
    memory[1].data[10] = 0x65;
    memory[1].data[11] = 0x87; // MSB

    std::list<uint32_t> lru = {1};
    std::unordered_map<uint32_t, std::list<uint32_t>::iterator> lru_map;
    lru_map[0x1000000] = lru.begin();

    cache_layer.set_memory(memory, lru, lru_map);
    addr.write(0x10000008);
    r.write(true);
    w.write(false);
    sc_start(10, SC_NS);
    r.write(false);

    assert_bool_layer("FullyAssociativeHit_Miss", false, miss.read());
    assert_equal_layer("FullyAssociativeHit_Data", 0x87654321, data.read());
    assert_bool_layer("FullyAssociativeHit_Ready", true, ready.read());
    // Überprüfe LRU-Update
    assert_equal_layer("FullyAssociativeHit_LRU", 1, cache_layer.lru_list.front());
}

void test_fully_associative_read_miss(CACHE_LAYER &cache_layer, sc_signal<uint32_t> &addr, sc_signal<bool> &r, sc_signal<bool> &w,
                                      const sc_signal<bool> &miss, const sc_signal<bool> &ready)
{
    cache_layer.mapping_strategy = 1; // Fully associative
    std::vector<CacheLine> memory(4, {0, false, std::vector<uint8_t>(16)});
    std::list<uint32_t> lru;
    std::unordered_map<uint32_t, std::list<uint32_t>::iterator> lru_map;

    cache_layer.set_memory(memory, lru, lru_map);
    addr.write(0x10000008);
    r.write(true);
    w.write(false);
    sc_start(10, SC_NS);
    r.write(false);

    assert_bool_layer("FullyAssociativeMiss_Miss", true, miss.read());
    assert_bool_layer("FullyAssociativeMiss_Ready", true, ready.read());
}

void test_fully_associative_invalid_offset(CACHE_LAYER &cache_layer, sc_signal<uint32_t> &addr, sc_signal<bool> &r, sc_signal<bool> &w)
{
    cache_layer.mapping_strategy = 1; // Fully associative
    std::vector<CacheLine> memory(4, {0, false, std::vector<uint8_t>(16)});
    cache_layer.set_memory(memory, {}, {});
    addr.write(0x1000000E);
    r.write(true);
    w.write(false);
    sc_start(10, SC_NS);

    assert_bool_layer("FullyAssociativeInvalidOffset", true, cache_layer.error);
}

void test_direct_mapped_write_hit(CACHE_LAYER &cache_layer, sc_signal<uint32_t> &addr, sc_signal<uint32_t> &wdata, sc_signal<bool> &r, sc_signal<bool> &w, sc_signal<bool> &miss, sc_signal<bool> &ready)
{
    cache_layer.mapping_strategy = 0; // Direct-Mapped
    std::vector<CacheLine> memory(4, {0, false, std::vector<uint8_t>(16)});
    memory[0].tag = 0x00010008 >> 6;
    memory[0].valid = true;
    // Initial data: 0x12345678 at offset 8
    memory[0].data[8] = 0x78;
    memory[0].data[9] = 0x56;
    memory[0].data[10] = 0x34;
    memory[0].data[11] = 0x12;

    cache_layer.set_memory(memory, {}, {});
    addr.write(0x00010008);  // Tag=0x400, Index=0, Offset=8
    wdata.write(0x87654321); // New data
    r.write(false);
    w.write(true);
    sc_start(10, SC_NS);
    w.write(false);

    assert_bool_layer("DirectMappedWriteHit_Miss", false, miss.read());
    assert_bool_layer("DirectMappedWriteHit_Ready", true, ready.read());
    assert_bool_layer("DirectMappedWriteHit_NoError", false, cache_layer.error);
    
    std::vector<uint8_t> expected_data(16, 0);
    expected_data[8] = 0x21; // LSB
    expected_data[9] = 0x43;
    expected_data[10] = 0x65;
    expected_data[11] = 0x87; // MSB
    assert_data("DirectMappedWriteHit_Data", expected_data, cache_layer.cache_memory[0].data);
}

void test_direct_mapped_write_miss(CACHE_LAYER &cache_layer, sc_signal<uint32_t> &addr, sc_signal<uint32_t> &wdata, sc_signal<bool> &r, sc_signal<bool> &w, sc_signal<bool> &miss, sc_signal<bool> &ready)
{
    cache_layer.mapping_strategy = 0; // Direct-Mapped
    std::vector<CacheLine> memory(4, {0, false, std::vector<uint8_t>(16)});
    memory[0].tag = 0x00020; // False Tag
    memory[0].valid = true;

    cache_layer.set_memory(memory, {}, {});
    addr.write(0x00010008); // Tag=0x00010, Index=0, Offset=8
    wdata.write(0x87654321);
    r.write(false);
    w.write(true);
    sc_start(10, SC_NS);
    w.write(false);

    assert_bool_layer("DirectMappedWriteMiss_Miss", true, miss.read());
    assert_bool_layer("DirectMappedWriteMiss_Ready", true, ready.read());
    assert_bool_layer("DirectMappedWriteMiss_NoError", false, cache_layer.error);

    std::vector<uint8_t> expected_data(16, 0); // zero-filled cacheline to check if data is not changed
    assert_data("DirectMappedWriteMiss_Data", expected_data, cache_layer.cache_memory[0].data);
}

void test_direct_mapped_write_invalid_offset(CACHE_LAYER &cache_layer, sc_signal<uint32_t> &addr, sc_signal<bool> &r, sc_signal<bool> &w)
{
    cache_layer.mapping_strategy = 0; // Direct-Mapped
    std::vector<CacheLine> memory(4, {0, false, std::vector<uint8_t>(16)});
    cache_layer.set_memory(memory, {}, {});
    addr.write(0x0001000E); // Offset=0xE, not valid
    r.write(false);
    w.write(true);
    sc_start(10, SC_NS);
    w.write(false);

    assert_bool_layer("DirectMappedWriteInvalidOffset", true, cache_layer.error);
}

void test_fully_associative_write_hit(CACHE_LAYER &cache_layer, sc_signal<uint32_t> &addr, sc_signal<uint32_t> &wdata, sc_signal<bool> &r, sc_signal<bool> &w, sc_signal<bool> &miss, sc_signal<bool> &ready)
{
    cache_layer.mapping_strategy = 1; // Fully-Associative
    std::vector<CacheLine> memory(4, {0, false, std::vector<uint8_t>(16)});
    memory[1].tag = 0x1000000;
    memory[1].valid = true;
    memory[1].data[8] = 0x78; // Initial Data: 0x12345678
    memory[1].data[9] = 0x56;
    memory[1].data[10] = 0x34;
    memory[1].data[11] = 0x12;

    std::list<uint32_t> lru = {1};
    std::unordered_map<uint32_t, std::list<uint32_t>::iterator> lru_map;
    lru_map[0x1000000] = lru.begin();

    cache_layer.set_memory(memory, lru, lru_map);
    addr.write(0x10000008);  // Tag=0x1000000, Offset=8
    wdata.write(0x87654321); // new data
    r.write(false);
    w.write(true);
    sc_start(10, SC_NS);
    w.write(false);

    assert_bool_layer("FullyAssociativeWriteHit_Miss", false, miss.read());
    assert_bool_layer("FullyAssociativeWriteHit_Ready", true, ready.read());
    assert_bool_layer("FullyAssociativeWriteHit_NoError", false, cache_layer.error);
    assert_equal_layer("FullyAssociativeWriteHit_LRU", 1, cache_layer.lru_list.front());

    std::vector<uint8_t> expected_data(16, 0);
    expected_data[8] = 0x21;
    expected_data[9] = 0x43;
    expected_data[10] = 0x65;
    expected_data[11] = 0x87;
    assert_data("FullyAssociativeWriteHit_Data", expected_data, cache_layer.cache_memory[1].data);
}

void test_fully_associative_write_miss(CACHE_LAYER &cache_layer, sc_signal<uint32_t> &addr, sc_signal<uint32_t> &wdata, sc_signal<bool> &r, sc_signal<bool> &w, sc_signal<bool> &miss, sc_signal<bool> &ready)
{
    cache_layer.mapping_strategy = 1; // Fully-Associative
    std::vector<CacheLine> memory(4, {0, false, std::vector<uint8_t>(16)});
    std::list<uint32_t> lru;
    std::unordered_map<uint32_t, std::list<uint32_t>::iterator> lru_map;

    cache_layer.set_memory(memory, lru, lru_map);
    addr.write(0x10000008); // Tag=0x1000000, Offset=8
    wdata.write(0x87654321);
    r.write(false);
    w.write(true);
    sc_start(10, SC_NS);
    w.write(false);

    assert_bool_layer("FullyAssociativeWriteMiss_Miss", true, miss.read());
    assert_bool_layer("FullyAssociativeWriteMiss_Ready", true, ready.read());
    assert_bool_layer("FullyAssociativeWriteMiss_NoError", false, cache_layer.error);

    std::vector<uint8_t> expected_data(16, 0);
    assert_data("FullyAssociativeWriteMiss_Data", expected_data, cache_layer.cache_memory[0].data);
}

void test_fully_associative_write_invalid_offset(CACHE_LAYER &cache_layer, sc_signal<uint32_t> &addr, sc_signal<bool> &r, sc_signal<bool> &w)
{
    cache_layer.mapping_strategy = 1; // Fully-Associative
    std::vector<CacheLine> memory(4, {0, false, std::vector<uint8_t>(16)});
    cache_layer.set_memory(memory, {}, {});
    addr.write(0x1000000E); // Offset=0xE, invalid
    r.write(false);
    w.write(true);
    sc_start(10, SC_NS);
    w.write(false);

    assert_bool_layer("FullyAssociativeWriteInvalidOffset", true, cache_layer.error);
}

void test_write_cacheline_direct_mapped(CACHE_LAYER &cache_layer)
{
    cache_layer.mapping_strategy = 0; // Direct-Mapped
    std::vector<uint8_t> mem_data(16, 0);
    mem_data[8] = 0x21; // Data: 0x87654321
    mem_data[9] = 0x43;
    mem_data[10] = 0x65;
    mem_data[11] = 0x87;

    cache_layer.set_memory(std::vector<CacheLine>(4, {0, false, std::vector<uint8_t>(16)}), {}, {});
    cache_layer.write_cacheline(0x00010008, mem_data); // Tag=0x400, Index=0

    assert_equal_layer("WriteDataDirectMapped_Tag", 0x00010008 >> 6, cache_layer.cache_memory[0].tag);
    assert_bool_layer("WriteDataDirectMapped_Valid", true, cache_layer.cache_memory[0].valid);
    assert_data("WriteDataDirectMapped_Data", mem_data, cache_layer.cache_memory[0].data);
}

void test_write_cacheline_fully_associative_not_full(CACHE_LAYER &cache_layer)
{
    cache_layer.mapping_strategy = 1;
    std::vector<CacheLine> memory(4, {0, false, std::vector<uint8_t>(16)});
    std::list<uint32_t> lru = {0};
    std::unordered_map<uint32_t, std::list<uint32_t>::iterator> lru_map;
    lru_map[0x2000000] = lru.begin();
    memory[0].tag = 0x2000000;
    memory[0].valid = true;
    cache_layer.set_memory(memory, lru, lru_map);
    cache_layer.size = 1; // only one cache line

    std::vector<uint8_t> mem_data(16, 0);
    mem_data[8] = 0x21; // Data: 0x87654321
    mem_data[9] = 0x43;
    mem_data[10] = 0x65;
    mem_data[11] = 0x87;

    cache_layer.write_cacheline(0x10000008, mem_data); // Tag=0x1000000

    assert_equal_layer("WriteDataFullyAssociativeNotFull_Size", 2, cache_layer.size);
    assert_equal_layer("WriteDataFullyAssociativeNotFull_LRUFront", 1, cache_layer.lru_list.front());
    assert_equal_layer("WriteDataFullyAssociativeNotFull_Tag", 0x10000008 >> 4, cache_layer.cache_memory[1].tag);
    assert_bool_layer("WriteDataFullyAssociativeNotFull_Valid", true, cache_layer.cache_memory[1].valid);
    assert_data("WriteDataFullyAssociativeNotFull_Data", mem_data, cache_layer.cache_memory[1].data);
    assert_equal_layer("WriteDataFullyAssociativeNotFull_LRUMap", 1, *cache_layer.lru_map[0x10000008 >> 4]);
}

void test_write_data_from_main_memory_fully_associative_full(CACHE_LAYER &cache_layer)
{
    cache_layer.mapping_strategy = 1;
    std::vector<CacheLine> memory(4, {0, false, std::vector<uint8_t>(16)});
    std::list<uint32_t> lru = {0, 1, 2, 3}; // LRU: 3
    std::unordered_map<uint32_t, std::list<uint32_t>::iterator> lru_map;
    for (uint32_t i = 0; i < 4; ++i)
    {
        memory[i].tag = 0x1000000 + i;
        memory[i].valid = true;
        lru_map[0x1000000 + i] = std::next(lru.begin(), i);
    }
    cache_layer.set_memory(memory, lru, lru_map);
    cache_layer.size = 4; // Full cache_layer

    std::vector<uint8_t> mem_data(16, 0);
    mem_data[8] = 0x21; // Data: 0x87654321
    mem_data[9] = 0x43;
    mem_data[10] = 0x65;
    mem_data[11] = 0x87;

    cache_layer.write_cacheline(0x20000008, mem_data); // Tag=0x2000000, LRU replace at index 3

    assert_equal_layer("WriteDataFullyAssociativeFull_Size", 4, cache_layer.size);
    assert_equal_layer("WriteDataFullyAssociativeFull_LRUFront", 3, cache_layer.lru_list.front());
    assert_equal_layer("WriteDataFullyAssociativeFull_Tag", 0x2000000, cache_layer.cache_memory[3].tag);
    assert_bool_layer("WriteDataFullyAssociativeFull_Valid", true, cache_layer.cache_memory[3].valid);
    assert_data("WriteDataFullyAssociativeFull_Data", mem_data, cache_layer.cache_memory[3].data);
    assert_equal_layer("WriteDataFullyAssociativeFull_LRUMap", 3, *cache_layer.lru_map[0x2000000]);
    assert_bool_layer("WriteDataFullyAssociativeFull_LRUMapErased", false, cache_layer.lru_map.count(0x1000003) > 0);
}

void test_write_cacheline_invalid_strategy(CACHE_LAYER &cache_layer)
{
    cache_layer.mapping_strategy = 2; // Ungültig
    std::vector<uint8_t> mem_data(16, 0);
    cache_layer.write_cacheline(0x00010008, mem_data);
    assert_bool_layer("WriteDataInvalidStrategy", true, cache_layer.error);
}

void run_cache_layer_tests()
{
    sc_clock clk("clk1", 10, SC_NS);
    sc_signal<uint32_t> addr, wdata;
    sc_signal<bool> r, w, miss, ready;
    sc_signal<uint32_t> data;
    CACHE_LAYER cache_layer("cache_layer", 0, 4, 16, 0, 0); // Default: direct-mapped
    cache_layer.clk(clk);
    cache_layer.addr(addr);
    cache_layer.wdata(wdata);
    cache_layer.r(r);
    cache_layer.w(w);
    cache_layer.miss(miss);
    cache_layer.ready(ready);
    cache_layer.data(data);
    cache_layer.test_mode = true; // Enable test mode for exception handling

    std::cout << "Running Direct-Mapped Read Tests...\n";
    test_direct_mapped_read_hit(cache_layer, addr, r, w, miss, ready, data);
    test_direct_mapped_read_miss(cache_layer, addr, r, w, miss, ready, data);
    test_direct_mapped_invalid_offset(cache_layer, addr, r, w);

    std::cout << "\nRunning Direct-Mapped Write Tests...\n";
    test_direct_mapped_write_hit(cache_layer, addr, wdata, r, w, miss, ready);
    test_direct_mapped_write_miss(cache_layer, addr, wdata, r, w, miss, ready);
    test_direct_mapped_write_invalid_offset(cache_layer, addr, r, w);

    std::cout << "\nRunning Fully Associative Tests...\n";
    test_fully_associative_read_hit(cache_layer, addr, r, w, miss, ready, data);
    test_fully_associative_read_miss(cache_layer, addr, r, w, miss, ready);
    test_fully_associative_invalid_offset(cache_layer, addr, r, w);

    std::cout << "\nRunning Fully-Associative Write Tests...\n";
    test_fully_associative_write_hit(cache_layer, addr, wdata, r, w, miss, ready);
    test_fully_associative_write_miss(cache_layer, addr, wdata, r, w, miss, ready);
    test_fully_associative_write_invalid_offset(cache_layer, addr, r, w);

    std::cout << "\nRunning Write Data from Main Memory Tests...\n";
    test_write_cacheline_direct_mapped(cache_layer);
    cache_layer.mapping_strategy = 1; // Reset for fully-associative tests
    test_write_cacheline_fully_associative_not_full(cache_layer);
    test_write_data_from_main_memory_fully_associative_full(cache_layer);
    test_write_cacheline_invalid_strategy(cache_layer);
}