#ifndef DEFAULT_H
#define DEFAULT_H

/*CHANGE AS NEEDED*/
enum DefaultSimulationValues {
    CYCLES           = 10000000 ,
    NUM_CACHE_LEVELS = 3        ,
    CACHE_LINE_SIZE  = 64       ,
    NUM_LINES_L1     = 512      ,
    NUM_LINES_L2     = 4096     , 
    NUM_LINES_L3     = 32768    ,
    LATENCY_CACHE_L1 = 8        ,
    LATENCY_CACHE_L2 = 16       ,
    LATENCY_CACHE_L3 = 32       ,
    MAPPING_STRATEGY = 1        ,
};

#endif // DEFAULT_H