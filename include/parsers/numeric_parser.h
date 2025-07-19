#ifndef NUMERIC_PARSER_H
#define NUMERIC_PARSER_H

#include <stdint.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#define INVALID_VALUE ((unsigned long)-1)

bool parse_unsigned_int8(const char* arg, uint8_t* out8, const char* name);
bool parse_unsigned_int32(const char* arg, uint32_t* out32, const char* name);
unsigned long validate_value_decimal(const char* arg, const char* name);

#endif // NUMERIC_PARSER_H