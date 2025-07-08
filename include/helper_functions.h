#ifndef HELPER_FUNCTIONS_H
#define HELPER_FUNCTIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include "request.h"
#include "default.h"
#include "debug.h"

void print_help();
char* read_file_to_buffer(const char* filename);
void print_requests(Request* requests, size_t size);

#endif // HELPER_FUNCTION_H