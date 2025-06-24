#include<systemc.h>
#include<string>

static sc_trace_file* tracefile = nullptr;

extern "C" void create_trace(const char* filename)
{
    tracefile = sc_create_vcd_trace_file(filename);
}

extern "C" void close_trace() {
    if (tracefile != nullptr){
        sc_close_vcd_trace_file(tracefile);
        tracefile = nullptr;
    }
}