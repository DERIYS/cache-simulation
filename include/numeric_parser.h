#ifndef NUMERIC_PARSER_H
#define NUMERIC_PARSER_H

#include <stdint.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#define IS_32BIT NULL
#define IS_8BIT  NULL

bool parseUnsignedInt(const char* arg, uint8_t* out8, uint32_t* out32, const char* name);

#endif // NUMERIC_PARSER_H