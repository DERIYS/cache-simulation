#ifndef MULTIPLEXER_HPP
#define MULTIPLEXER_HPP

#include <systemc>
#include <vector>
#include <stdint.h>
using namespace sc_core;


SC_MODULE(MULTIPLEXER_I32) {
  std::vector<sc_in<uint32_t>> in;
  sc_in<uint8_t> select;

  std::vector<sc_out<uint32_t>> out;

  SC_CTOR(MULTIPLEXER_I32);
  MULTIPLEXER_I32(sc_module_name name, uint8_t fanIn, uint8_t fanOut) : sc_module(name), in(fanIn), out(fanOut) {
    SC_THREAD(behaviour);
    sensitive << select;

    for(int i = 0; i < fanIn; i++) {
      sensitive << in[i];
    }
  }

  void behaviour() {
    while(true) {
      uint8_t selectValue = select.read();
      int32_t selected = selectValue < in.size() ? in[selectValue].read() : 0;
      for (size_t i = 0; i < out.size(); i++) {
        out[i].write(selected);
      }
      wait();
    }
  }
};

#endif // MULTIPLEXER_HPP