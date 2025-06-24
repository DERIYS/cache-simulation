#include "../include/csv_parser.h"

//This function was inspired by MiniAssembler homework submission 
char* split_next_line(const char* content, char* type, char* address, char* data) {
    const char* newline = strchr(content, '\n');
    size_t line_len = newline ? (size_t)(newline - content) : strlen(content);

    char* line_copy = strndup(content, line_len);
    if (!line_copy) return NULL;

    const char* delimiters = " ,";

    char* token = strtok(line_copy, delimiters);
    if (token) {
        strcpy(type, token);
    } 
    else {
        printf("Failed to parse type\n");
        free(line_copy);
        return PARSE_ERROR;
    }

    token = strtok(NULL, delimiters);
    if (token) {
        strcpy(address, token);
    }
    else { 
        printf("Failed to parse address\n");
        free(line_copy);
        return PARSE_ERROR;
    }

    token = strtok(NULL, delimiters);
    if (token) {
        strcpy(data, token);
    } 
    else {
        *data = '\0';
    }

    token = strtok(NULL, delimiters);
    if (token)
    {
        printf("Unexpected extra data\n");
        free(line_copy);
        return PARSE_ERROR;
    }

    free(line_copy);    

    if (!newline) return NULL;

    return (char*)(newline + 1);
}

unsigned long countRequests(char* content)
{
    size_t requestsCount = 0L;
    char* requests = strdup(content);
    char* ptr = requests;

    while (*ptr){
        if (*ptr == '\n'){
            requestsCount++;
        }
        ptr++;
    }

    if (ptr != content && *(ptr-1) != '\n')
    {
        requestsCount++;
    }

    free(requests);
    return requestsCount;
}

uint32_t validateValue(char* someValue)
{
    char* isEnd;
    int value;
    if (strncmp(someValue, "0x", 2) == 0){
        value = strtol(someValue,&isEnd,16);
    } 
    else {
        value = strtol(someValue,&isEnd,10);
    }
    if (*isEnd != '\0')
    {
        printf("Failed to parse value\n");
        return VALUE_ERROR;
    }

    return (uint32_t)value;
}

struct Request formSingleRequest(char* type, char* address, char* data, bool* ok)
{
    struct Request req = {0};
    *ok = false;
    bool isR = false;

    if (strncmp(type,"W",1) == 0){
        req.w = 1;
    }
    else if (strncmp(type,"R",1) == 0){
        req.w = 0;
        isR   = true;
    }
    else {
        printf("Invalid type\n");
        return req;
    }

    uint32_t req_address = validateValue(address);
    if (req_address == VALUE_ERROR){
        printf("Invalid address\n");
        return req;
    } else {
        req.addr = req_address;
    }

    uint32_t req_data;
    if (isR && *data != '\0'){
        printf("Invalid data --- no data expected by read\n");
        return req;
    } else if (isR) {
        //nothing?  
    } else {
        req_data = validateValue(data);

        if (req_data == VALUE_ERROR){
            printf("Invalid data\n");
            return req;
        }
        else {
            req.data = req_data;
        }
    }
    *ok = true;
    return req;
}

int formRequests(char* content, struct Request* requests)
{

    char* type    = (char*) calloc(2, sizeof(char));
    char* address = (char*) calloc(21,sizeof(char));
    char* data    = (char*) calloc(21,sizeof(char));

    size_t request_counter = 0;
    while(content){
        content = split_next_line(content, type, address, data);

        if (content == PARSE_ERROR){
            free(type);
            free(address);
            free(data);
            return -1;
        }

        bool ok = false;
        struct Request request = formSingleRequest(type,address,data,&ok);
        if (!ok) {
            printf("Failed to form a request\n");
            free(type);
            free(address);
            free(data);
            return -1;
        }
        requests[request_counter++] = request;
    }

    free(type);
    free(address);
    free(data);

    return 0;
}