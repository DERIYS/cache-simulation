#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>

#include "../include/simulation.h"
#include "../include/csv_parser.h"

extern void create_trace(const char* filename);
extern void close_trace();

int main(int argc, char** argv)
{
    static struct option long_options[] = {
        {"cycles"          , required_argument, 0, 'c'},
        {"tf"              , optional_argument, 0, 'f'},
        {"help"            , optional_argument, 0, 'h'},
        {"cacheline-size"  , optional_argument, 0, 'C'},
        {"num-lines-l1"    , optional_argument, 0, '1'},
        {"num-lines-l2"    , optional_argument, 0, '2'},
        {"num-lines-l3"    , optional_argument, 0, '3'},
        {"latency-cache-l1", optional_argument, 0, 'l'},
        {"latency-cache-l2", optional_argument, 0, 'a'},
        {"latency-cache-l3", optional_argument, 0, 't'},
        {"num-cache-levels", optional_argument, 0, 'L'},
        {"mapping-strategy", optional_argument, 0, 'S'},
        {0                 , 0                , 0,  0 }
    };   

    uint32_t * cycles           = NULL;
    uint32_t * cacheLineSize    = NULL;
    uint32_t * numLinesL1       = NULL;
    uint32_t * numLinesL2       = NULL;
    uint32_t * numLinesL3       = NULL;
    uint32_t * latencyCacheL1   = NULL;
    uint32_t * latencyCacheL2   = NULL;
    uint32_t * latencyCacheL3   = NULL;
    uint8_t  * numCacheLevels   = NULL;
    uint8_t  * mappingStrategy  = NULL;

    printf("STAGE 0\n");

    int opt;
    int option_index = 0;
    while ((opt = getopt_long(argc,argv, "c:f:h:C:1:2:3:l:a:t:L:S", long_options, &option_index)) != -1)
    {

        switch (opt)
        {
            //STANDART OPTIONS
            case 'c':
                //TODO
                printf("Cycles set\n");
                break;
            case 'f':
                //TODO
                printf("Tracefile set\n");
                break;
            case 'h':
                //TODO
                printf("Help\n");
                return 0;

            //ADVANCED OPTIONS
            case 'C':
                //TODO
                printf("Cacheline size set\n");
                break;
            case '1':
                //TODO
                printf("Cache L1 lines set\n");
                break;
            case '2':
                //TODO
                printf("Cache L2 lines set\n");
                break;
            case '3':
                //TODO
                printf("Cache L3 lines set\n");
                break;
            case 'l':
                //TODO
                printf("Cache L1 latency set\n");
                break;
            case 'a':
                //TODO
                printf("Cache L2 latency set\n");
                break;
            case 't':
                //TODO
                printf("Cache L3 latency set\n");
                break;
            case 'L':
                printf("Cache levels set\n");
                break;
            case 'S':
                printf("Cache levels set\n");
                break;
            case '?':
                //TODO
                return -1;
        }
    }

    printf("STAGE 1\n");

    if (optind > argc) {
        //TODO
        //no input file
        printf("No input file\n");
        return -1;
    } 
    
    const char *filename = argv[optind];

    struct stat sb;
    if (stat(filename, &sb) == -1){
        printf("Access denied");
        return EACCES;
    }

    if (!S_ISREG(sb.st_mode))
    {
        printf("Not a regular file");
        return EISDIR;
    }

    FILE *csv_file = fopen(filename, "r");
    if (!csv_file) {
        printf("Error while opening file");
        return EPERM;
    }   

    printf("STAGE 2\n");

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

    printf("STAGE 3\n");

    size_t requests_size = countRequests(content);

    struct Request* requests = (struct Request*) calloc(requests_size, sizeof(struct Request));

    printf("STAGE 4\n");

    int err;
    err = formRequests(content, requests);
    if (err != 0) {
        printf("Failed to parse CSV\n");
        return EPERM;
    }

    printf("STAGE 5\n");

    fclose(csv_file);

    free(requests);

    for (size_t i = 0; i < requests_size; i++) {
        printf("Request %zu: type=%s, addr = 0x%08X, data=0x%08X\n", i,
            requests[i].w ? "W" : "R",
            requests[i].addr,
            requests[i].data);
    }

    printf("Success\n");

    return 0;
}