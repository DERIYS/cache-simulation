#ifndef LEGACY_H
#define LEGACY_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>   

#ifdef __cplusplus
extern "C" {
#endif

char* uint32_to_string(uint32_t value);
char* uint8_to_string(uint8_t value);

#ifdef __cplusplus
}
#endif

#endif // LEGACY_H