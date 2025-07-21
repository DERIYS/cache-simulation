#ifndef CSV_PARSER_H
#define CSV_PARSER_H

/* This macro ensures that debug.h uses appropriate DEBUG_PRINT function */
#define ENABLE_DEBUG

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "../structs/request.h"
#include "../structs/debug.h"

#define PARSE_ERROR ((char*)-1)
#define VALUE_ERROR (uint32_t)(-1)

char* split_next_line(const char* content, char* type, char* address, char* data);
Request form_single_request(char* type, char* address, char* data, bool* ok);
unsigned long count_requests(char* content);
int form_requests(char* content, Request* requests);
uint32_t validate_value(char* value);

#endif // CSV_PARSER_H