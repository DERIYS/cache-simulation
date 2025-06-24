#ifndef CSV_PARSER_H
#define CSV_PARSER_H

#include "simulation.h"
#include <stdio.h>
#include <cstring>
#include <cstdlib>

#define PARSE_ERROR ((char*)-1)
#define VALUE_ERROR (uint32_t)(-1)

char* split_next_line(const char* content, char* type, char* address, char* data);
struct Request formSingleRequest(char* type, char* address, char* data, bool* ok);
unsigned long countRequests(char* content);
int formRequests(char* content, struct Request* requests);
uint32_t validateValue(char* value);

#endif // CSV_PARSER_H