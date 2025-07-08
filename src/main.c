#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sysexits.h>
#include <sys/stat.h>

#include "../include/request.h"
#include "../include/default.h"
#include "../include/simulation.hpp"
#include "../include/csv_parser.h"
#include "../include/numeric_parser.h"
#include "../include/helper_functions.h"
#include "../include/debug.h"

/* Debug flag */
bool debug = false;

int main(int argc, char** argv)
{
    /* Supported long options for CLI parsing */
    static struct option long_options[] = {
        {"cycles"          , required_argument, 0, 'c'},
        {"tf"              , required_argument, 0, 'f'},
        {"help"            , no_argument,       0, 'h'},
        {"cacheline-size"  , required_argument, 0, 'C'},
        {"num-lines-l1"    , required_argument, 0, 'L'},
        {"num-lines-l2"    , required_argument, 0, 'M'},
        {"num-lines-l3"    , required_argument, 0, 'N'},
        {"latency-cache-l1", required_argument, 0, 'l'},
        {"latency-cache-l2", required_argument, 0, 'm'},
        {"latency-cache-l3", required_argument, 0, 'n'},
        {"num-cache-levels", required_argument, 0, 'e'},
        {"mapping-strategy", required_argument, 0, 'S'},
        {"debug"           , no_argument      , 0, 'd'},
        {0                 , 0                , 0,  0 }
    };   

    /* Default simultaion values */
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

    /* Parse CLI options using getopt_long.
       Supports both long (--cycles, --tf) and short (-c, -f) options.  */
    int opt;
    int option_index = 0;
    while ((opt = getopt_long(argc,argv, "c:f:hC:L:M:N:l:m:n:e:S:d", long_options, &option_index)) != -1)
    {

        /* Handle each option using its long/short flag. 
           All numeric inputs are validated and rejected in the case of negative values or invalid formats.
           Programm exits with meaningfull error. */

        /* Attention: following options parsing for numeric values heavily relies on parseUnsignedInt function.
                      For a better understanding of its usage and behaviour please check src/numeric_parser.c  */

        switch (opt)
        {
            /* Standart options */

            /* Parse and validate cycles, and pass it to simulation parametrs */
            case 'c':

                /* Passes argument for verification, cycles as uint32, where the argument will be stored, and string indicating error, if needed */
                if (!parseUnsignedInt(optarg, IS_32BIT, &cycles, "cycles value")) {
                    return EX_DATAERR;
                }

                DEBUG_PRINT("Cycles set\n");
                break;
            
            /* Tracefile specified. It should be created during simulation */
            case 'f':
                traceFileName = optarg;
                DEBUG_PRINT("Tracefile set\n");
                break;

            /* Help flag. When this option is set, print helpful message for application usage */
            case 'h':

                print_help();

                return EXIT_SUCCESS;

            /* ADVANCED OPTIONS */

            /* Parse and validate cache line size, and pass it to simulation paramets afterwards */
            case 'C':

                /* Passes argument for verification, cachelineSize as uint32, where the argument will be stored, and string indicating error, if needed */
                if (!parseUnsignedInt(optarg, IS_32BIT, &cachelineSize, "cache line size")) {
                    return EX_DATAERR;
                }

                DEBUG_PRINT("Cacheline size set\n");
                break;

            /* Parse and validate cache L1 number of lines, and pass it to simulation paramets afterwards */
            case 'L':
                
                /* Passes argument for verification, numLinesL1 as uint32, where the argument will be stored, and string indicating error, if needed */
                if (!parseUnsignedInt(optarg, IS_32BIT, &numLinesL1, "cache L1 line value")) {
                    return EX_DATAERR;
                }
          
                DEBUG_PRINT("Cache L1 lines set\n");
                break;

            /* Parse and validate cache L2 number of lines, and pass it to simulation paramets afterwards */
            case 'M':
                
                /* Passes argument for verification, numLinesL2 as uint32, where the argument will be stored, and string indicating error, if needed */
                if (!parseUnsignedInt(optarg, IS_32BIT , &numLinesL2, "cache L2 line value")) {
                    return EX_DATAERR;
                }

                DEBUG_PRINT("Cache L2 lines set\n");
                break;
        
            /* Parse and validate cache L3 number of lines, and pass it to simulation paramets afterwards */
            case 'N':
                
                /* Passes argument for verification, numLinesL3 as uint32, where the argument will be stored, and string indicating error, if needed */
                if (!parseUnsignedInt(optarg, IS_32BIT ,&numLinesL3, "cache L3 line value")) {
                    return EX_DATAERR;
                }

                DEBUG_PRINT("Cache L3 lines set\n");
                break;

            /* Parse and validate cache L1 latency, and pass it to simulation paramets afterwards */
            case 'l':
                
                /* Passes argument for verification, latencyCacheL1 as uint32, where the argument will be stored, and string indicating error, if needed */
                if (!parseUnsignedInt(optarg, IS_32BIT, &latencyCacheL1, "cache L1 latency value")) {
                    return EX_DATAERR;
                }

                DEBUG_PRINT("Cache L1 latency set\n");
                break;

            /* Parse and validate cache L2 latency, and pass it to simulation paramets afterwards */
            case 'm':
                
                /* Passes argument for verification, latencyCacheL2 as uint32, where the argument will be stored, and string indicating error, if needed */
                if (!parseUnsignedInt(optarg, IS_32BIT,  &latencyCacheL2, "cache L2 latency value")) {
                    return EX_DATAERR;
                }

                DEBUG_PRINT("Cache L2 latency set\n");
                break;

            /* Parse and validate cache L3 latency, and pass it to simulation paramets afterwards */
            case 'n':

                /* Passes argument for verification, latencyCache3 as uint32, where the argument will be stored, and string indicating error, if needed */
                if (!parseUnsignedInt(optarg, IS_32BIT, &latencyCacheL3, "cache L3 latency value")) {
                    return EX_DATAERR;
                }

                DEBUG_PRINT("Cache L3 latency set\n");
                break;
            
            /* Parse and validate number of cache levels, and pass it to simulation paramets afterwards */
            case 'e':

                /* Passes argument for verification, numCacheLevels as uint8, where the argument will be stored, and string indicating error, if needed */
                if (!parseUnsignedInt(optarg, &numCacheLevels, IS_8BIT , "number of cache levels")) {
                    return EX_DATAERR;
                }

                if (numCacheLevels < 1 || numCacheLevels > 3) {
                    fprintf(stderr, "Number of caches is between 1 and 3.\n");
                    return EX_DATAERR;
                }

                DEBUG_PRINT("Cache levels set\n");
                break;
            
            /* Parse and validate mapping strategy, and pass it to simulation paramets afterwards */
            case 'S':
                
                /* Passes argument for verification, mappingStrategy as uint8, where the argument will be stored, and string indicating error, if needed */
                if (!parseUnsignedInt(optarg, &mappingStrategy, IS_8BIT ,"mapping strategy")) {
                    return EX_DATAERR;
                }

                if (mappingStrategy < 0 || mappingStrategy > 1){
                    fprintf(stderr,"Mapping strategy is either 0 (Dirrect-mapped) or 1 (Fully-associative).\n");
                    return EX_DATAERR;
                }

                DEBUG_PRINT("Mapping strategy set\n");
                break;

            /* Set debuggers mode */
            case 'd':

                /* Set debug flag as true for debugging purposes */
                debug = true;

                DEBUG_PRINT("Debug set\n");
                break;

            /* Unrecognized option */
            case '?':
                if (optopt) {
                    fprintf(stderr, "Unknown option '-%c'. Please use --help or -h to see valid options.\n", optopt);
                } 
                else {
                    fprintf(stderr, "Invalid command argument. Please use --help or -h to see valid options.\n");
                }
                return EX_USAGE;
        }
    }

    /* No input file specified */
    if (optind >= argc) {
        fprintf(stderr, "No input file specified. Please run with --help.\n");
        return EX_USAGE;
    } 
    
    /* Get filename from last argument */
    const char *filename = argv[optind];

    /* Try to read into content buffer contents from file, specified by filename*/
    char* content = read_file_to_buffer(filename);
    if (!content){
        /*Failed to write in buffer*/
        return EPERM;
    }

    /* Count how many requests in order to allocate memory accordingly */
    uint32_t requests_size = countRequests(content);

    /* Allocate memory for requests */
    Request* requests = (Request*) calloc(requests_size, sizeof(Request));

    /* Try to form requests */
    int err;
    err = formRequests(content, requests);
    if (err != 0) {
        /* In the case of error, cleanup and return with an error*/
        free(requests);
        free(content);
        return EPERM;
    }

    /* If debug mode enabled, print requests to the console output */
    debug ? (void)print_requests(requests, requests_size) : (void)0;
    
    /* Run C++ SystemC simulation */
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

    /* Normal cleanup */
    free(requests);
    free(content);


    /* Programm ran successfuly*/
    return EXIT_SUCCESS;
}

/* Linker satisfier */
int sc_main(int argc, char* argv[])
{
    return 0;
}