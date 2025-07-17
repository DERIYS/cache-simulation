#ifndef REQUEST_H
#define REQUEST_H

#include <stdint.h>

typedef struct { 
    uint32_t    addr;
    uint32_t    data;
    uint8_t        w;
} Request;

#endif // REQUEST_H