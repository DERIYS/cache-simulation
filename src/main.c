#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>

#include "../include/structs.h"
#include "../include/simulation.hpp"
#include "../include/csv_parser.h"

extern void create_trace(const char* filename);
extern void close_trace();

int main(int argc, char** argv)
{
    static struct option long_options[] = {
        {"cycles"          , required_argument, 0, 'c'},
        {"tf"              , optional_argument, 0, 'f'},
        {"help"            , no_argument, 0, 'h'},
        {"cacheline-size"  , required_argument, 0, 'C'},
        {"num-lines-l1"    , required_argument, 0, '1'},
        {"num-lines-l2"    , required_argument, 0, '2'},
        {"num-lines-l3"    , required_argument, 0, '3'},
        {"latency-cache-l1", required_argument, 0, 'l'},
        {"latency-cache-l2", required_argument, 0, 'a'},
        {"latency-cache-l3", required_argument, 0, 't'},
        {"num-cache-levels", required_argument, 0, 'L'},
        {"mapping-strategy", required_argument, 0, 'S'},
        {0                 , 0                , 0,  0 }
    };   

    uint32_t  cycles           = CYCLES;
    uint32_t  cachelineSize    = CACHE_LINE_SIZE;
    uint32_t  numLinesL1       = NUM_LINES_L1;
    uint32_t  numLinesL2       = NUM_LINES_L2;
    uint32_t  numLinesL3       = NUM_LINES_L3;
    uint32_t  latencyCacheL1   = LATENCY_CACHE_L1;
    uint32_t  latencyCacheL2   = LATENCY_CACHE_L2;
    uint32_t  latencyCacheL3   = LATENCY_CACHE_L3;
    uint8_t   numCacheLevels   = NUM_CACHE_LEVELS;
    uint8_t   mappingStrategy  = MAPPING_STRATEGY;

    char*     traceFileName    = NULL;

    int opt;
    int option_index = 0;
    while ((opt = getopt_long(argc,argv, "c:f:h:C:1:2:3:l:a:t:L:S:", long_options, &option_index)) != -1)
    {
        char* endptr;

        switch (opt)
        {
            //STANDART OPTIONS
            case 'c':
                cycles = (uint32_t) strtoul(optarg, &endptr, 10);

                if (*endptr != '\0' || cycles <= 0) {
                    fprintf(stderr, "Invalid cycles value: %s\n", optarg);
                    return 1;
                }

                printf("Cycles set\n");
                break;
            case 'f':
                traceFileName = optarg;
                printf("Tracefile set\n");
                break;
            case 'h':
                //TODO
                printf("Help\n");
                return 0;

            //ADVANCED OPTIONS
            case 'C':
                cachelineSize = (uint32_t) strtoul(optarg, &endptr, 10);

                if (*endptr != '\0' || cachelineSize <= 0) {
                    fprintf(stderr, "Invalid cache line size value: %s\n", optarg);
                    return 1;
                }          

                printf("Cacheline size set\n");
                break;
            case '1':
                numLinesL1 = (uint32_t) strtoul(optarg, &endptr, 10);

                if (*endptr != '\0' || numLinesL1 <= 0) {
                    fprintf(stderr, "Invalid number of lines L1: %s\n", optarg);
                    return 1;
                }
          
                printf("Cache L1 lines set\n");
                break;
            case '2':
                numLinesL2 = (uint32_t) strtoul(optarg, &endptr, 10);

                if (*endptr != '\0' || numLinesL2 <= 0) {
                    fprintf(stderr, "Invalid number of lines L2: %s\n", optarg);
                    return 1;
                }

                printf("Cache L2 lines set\n");
                break;
            case '3':
                numLinesL3 = (uint32_t) strtoul(optarg, &endptr, 10);

                if (*endptr != '\0' || numLinesL3 <= 0) {
                    fprintf(stderr, "Invalid number of lines L3: %s\n", optarg);
                    return 1;
                }

                printf("Cache L3 lines set\n");
                break;
            case 'l':
                latencyCacheL1 = (uint32_t) strtoul(optarg, &endptr, 10);

                if (*endptr != '\0' || latencyCacheL1 <= 0) {
                    fprintf(stderr, "Invalid L1 latency value: %s\n", optarg);
                    return 1;
                }

                printf("Cache L1 latency set\n");
                break;
            case 'a':
                latencyCacheL2 = (uint32_t) strtoul(optarg, &endptr, 10);

                if (*endptr != '\0' || latencyCacheL2 <= 0) {
                    fprintf(stderr, "Invalid L2 latency value: %s\n", optarg);
                    return 1;
                }                 
                printf("Cache L2 latency set\n");
                break;
            case 't':
                latencyCacheL3 = (uint32_t) strtoul(optarg, &endptr, 10);

                if (*endptr != '\0' || latencyCacheL3 <= 0) {
                    fprintf(stderr, "Invalid L3 latency value: %s\n", optarg);
                    return 1;
                }   

                printf("Cache L3 latency set\n");
                break;
            case 'L':
                numCacheLevels = (uint8_t) strtoul(optarg, &endptr, 10);

                if (*endptr != '\0' || numCacheLevels <= 0) {
                    fprintf(stderr, "Invalid number of cache levels %s\n", optarg);
                    return 1;
                }

                printf("Cache levels set\n");
                break;
            case 'S':
                unsigned long tmp = strtoul(optarg, &endptr, 10);  

                if (*endptr != '\0' || tmp > 2 || tmp <= 0) {
                    fprintf(stderr, "Invalid value for mapping strategy %s\n", optarg);
                    return 1;
                }
                mappingStrategy = (uint8_t) tmp;

                printf("Mapping strategy set\n");
                break;
            case '?':
                //TODO
                return -1;
        }
    }

    if (optind >= argc) {
        //TODO
        //no input file
        printf("No input file\n");
        return -1;
    } 
    
    const char *filename = argv[optind];

    struct stat sb;
    if (stat(filename, &sb) == -1){
        printf("Access denied\n");
        return EACCES;
    }

    if (!S_ISREG(sb.st_mode))
    {
        printf("Not a regular file\n");
        return EISDIR;
    }

    FILE *csv_file = fopen(filename, "r");
    if (!csv_file) {
        printf("Error while opening file\n");
        return EPERM;
    }   

    char ch;
    char * content = NULL;
    size_t size = 0;
    while ((ch = fgetc(csv_file)) != EOF){
        content = (char*) realloc(content,size+2);
        if (!content){
            fclose(csv_file);
            return 1; //?
        }
        content[size++] = ch;
    }
    content[size] = '\0';

    uint32_t requests_size = countRequests(content);

    struct Request* requests = (struct Request*) calloc(requests_size, sizeof(struct Request));

    int err;
    err = formRequests(content, requests);
    if (err != 0) {
        printf("Failed to parse CSV\n");
        return EPERM;
    }

    fclose(csv_file);

    for (size_t i = 0; i < requests_size; i++) {
        printf("Request %zu: type=%s, addr = 0x%08X, data=0x%08X\n", i,
            requests[i].w ? "W" : "R",
            requests[i].addr,
            requests[i].data);
    }
    
    run_simulation(
                cycles,
         traceFileName, /*tracefile*/ 
        numCacheLevels,
         cachelineSize,
            numLinesL1,
            numLinesL2,
            numLinesL3,
        latencyCacheL1,
        latencyCacheL2,
        latencyCacheL3,
       mappingStrategy,
         requests_size,
              requests
    );

    free(requests);
    free(content);

    printf("Success\n");

    return EXIT_SUCCESS;
}

int sc_main(int argc, char* argv[])
{
    return 0;
}