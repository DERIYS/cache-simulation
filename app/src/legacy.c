#include "../include/legacy.h"

char* uint32_to_string(uint32_t value)
{
    char* str = (char*) malloc(11);
    if (!str) return NULL;

    snprintf(str, 11, "%u", value);
    return str;
}

char* uint8_to_string(uint8_t value)
{
    char* str = (char*) malloc(4);
    if (!str) return NULL;

    snprintf(str, 11, "%u", value);
    return str;
}