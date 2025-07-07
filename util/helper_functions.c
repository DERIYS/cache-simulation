#include "../include/helper_functions.h"

/* 
   * @brief         Prints all of the possible options to run the program 
   *
   * @return        void
*/
void print_help() 
{
    printf(
        "Usage: ./cache [OPTIONS] input.csv\n"
        "\n"
        "Simulate a cache system based on memory access requests from a CSV file.\n\n"
        "Required:\n"
        "  input.csv                 CSV file with memory requests.\n\n"
        "Standard options:\n"
        "  -c, --cycles NUM          Number of simulation cycles [default: %u]\n"
        "  -f, --tf FILE             Output trace file (optional)\n"
        "  -h, --help                Show this help message and exit\n\n"
        "Advanced options:\n"
        "  -C, --cacheline-size     |  Cache line size in bytes (default: %u)\n"
        "  -L, --num-lines-l1       |  Number of lines in L1 cache (default: %u)\n"
        "  -M, --num-lines-l2       |  Number of lines in L2 cache (default: %u)\n"
        "  -N, --num-lines-l3       |  Number of lines in L3 cache (default: %u)\n"
        "  -l, --latency-cache-l1   |  Latency of L1 cache in cycles (default: %u)\n"
        "  -m, --latency-cache-l2   |  Latency of L2 cache in cycles (default: %u)\n"
        "  -n, --latency-cache-l3   |  Latency of L3 cache in cycles (default: %u)\n"
        "  -e, --num-cache-levels   |  Number of cache levels (1–3) (default: %u)\n"
        "  -S, --mapping-strategy   |  Cache mapping strategy (0=Direct-mapped, 1=Fully associative.) (default: %u)\n\n"
        "Examples:\n"
        "  ./cache -c 1000 -f trace.txt --num-lines-l1 64 --mapping-strategy 1 input.csv\n",
        CYCLES,
        CACHE_LINE_SIZE,
        NUM_LINES_L1,
        NUM_LINES_L2,
        NUM_LINES_L3,
        LATENCY_CACHE_L1,
        LATENCY_CACHE_L2,
        LATENCY_CACHE_L3,
        NUM_CACHE_LEVELS,
        MAPPING_STRATEGY
    );
}

/*
   * @brief               Debug function to print all of the requests to console output
   *
   * @param requests      Pointer to requests array
   * @param size          Size of requests array 
   * 
   * @return        void
*/
void print_requests(struct Request* requests, size_t size){
    for (size_t i = 0; i < size; i++) {
        DEBUG_PRINT("Request %zu: type=%s, addr = 0x%08X, data=0x%08X\n", i,
            requests[i].w ? "W" : "R",
            requests[i].addr,
            requests[i].data);
    }
}

/*
   * @brief              
   *
   * @param filename    The path to the file to read.  
   * 
   * @return            A pointer to a null-terminated buffer containing the file contents,
   *                    or NULL if an error occurs (file not accessible, not a regular file,
   *                    I/O error, or memory allocation failure).
*/
char* read_file_to_buffer(const char *filename)
{
    struct stat sb;

    /* Check if file exists */
    if (stat(filename, &sb) == -1){
        fprintf(stderr,"Access denied\n");
        return NULL;
    }
    
    /* Check if is a regular file */
    if (!S_ISREG(sb.st_mode))
    {
        fprintf(stderr,"Not a regular file\n");
        return NULL;
    }

    /* Try to open a file for reading */
    FILE *csv_file = fopen(filename, "r");
    if (!csv_file) {
        fprintf(stderr, "Error while opening file\n");
        return NULL;
    }   
    
    /* Read the file charachter by charachter into a buffer */
    char ch;
    char * content = NULL;
    size_t size = 0;
    while ((ch = fgetc(csv_file)) != EOF){
        /* Resize buffer to hold one more charachter + '\0' */
        char *tmp =  realloc(content,size+2);
        if (!tmp){
            free(content);
            fclose(csv_file);
            fprintf(stderr, "Memory allocation failed\n");
            return NULL; 
        }
        content = tmp;
        content[size++] = ch;
    }

    /* If some I/O error occured during reading */
    if (ferror(csv_file)){
        free(content);
        fclose(csv_file);
        fprintf(stderr, "Error reading file %s\n", filename);
        return NULL;
    }

    fclose(csv_file);

    /* Null-terminate the content buffer */
    if (content) {
        content[size] = '\0';
    } else {
        /* File is empty */
        fprintf(stderr, "File is empty\n");
        return NULL;
    }

    return content;
}