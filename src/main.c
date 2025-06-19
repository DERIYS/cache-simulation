#include <getopt.h>
#include <stdio.h>

#include "../include/simulation.hpp"

int main(int argc, char** argv)
{
    static struct option long_options[] = {
        {"cycles", required_argument, 0,'c'},
        {"tf"    , optional_argument, 0,'t'},
        {"help"  , optional_argument, 0,'h'},
        {0       , 0                , 0, 0 }
    };   

    int opt;
    int option_index = 0;
    while ((opt = getopt_long(argc,argv, "c:t:h", long_options, &option_index)) != -1)
    {

        switch (opt)
        {
            case 'c':
                //TODO
                printf("Cycles set\n");
                break;
            case 't':
                //TODO
                printf("Tracefile set\n");
                break;
            case 'h':
                //TODO
                printf("help\n");
                return 0;
            case '?':
                //TODO
                return -1;
        }
    }

    if (optind < argc) {
        //TODO
        //input file
        printf("Input file set\n");
    } 
    else {
        //TODO
        //no input file
        printf("No input file\n");
        return -1;
    }

    return 0;
}