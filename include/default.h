#ifndef DEFAULT_H
#define DEFAULT_H

//CHANGE AS NEEDED
enum DefaultSimulationValues {
    CYCLES           = 1000,
    NUM_CACHE_LEVELS = 3   ,
    CACHE_LINE_SIZE  = 8   ,
    NUM_LINES_L1     = 8   ,
    NUM_LINES_L2     = 16  ,
    NUM_LINES_L3     = 32  ,
    LATENCY_CACHE_L1 = 10  ,
    LATENCY_CACHE_L2 = 20  ,
    LATENCY_CACHE_L3 = 30  ,
    MAPPING_STRATEGY = 1   ,
};

#endif // DEFAULT_H