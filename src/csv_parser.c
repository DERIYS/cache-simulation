#include "../include/csv_parser.h"

/*
   * @brief               Splits the content by line and returns pointer to new line in content
   *
   * @param content       Pointer to content to read from
   * @param type          Buffer for type of a request (either R or W)
   * @param address       Buffer for address of a request
   * @param data          Buffer for data of a request 
   * 
   * @return              Pointer to the next line in content or NULL, if there is no more lines
   * 
   * @copyright           This function was inspired by MiniAssembler homework submission 
*/
char* split_next_line(const char* content, char* type, char* address, char* data) {
    /* Find the end of the current line */
    const char* newline = strchr(content, '\n');
    size_t line_len = newline ? (size_t)(newline - content) : strlen(content);

    /* Create a temporary copy of the line for tokenization */
    char* line_copy = strndup(content, line_len);
    if (!line_copy) return NULL;

    /* Count commas to ensure correct CSV format */
    int comma_count = 0;
    for (size_t i = 0; i < line_len; i++){
        if (content[i] == ',') comma_count++;
    }
    if (comma_count < 2){
        fprintf(stderr, "Missing fields: less than 3 columns\n");
        free(line_copy);
        return PARSE_ERROR;
    }

    /* Tokenization using space and comma delimetrs */
    const char* delimiters = " ,";

    /* Extract type */
    char* token = strtok(line_copy, delimiters);
    if (token) {
        strcpy(type, token);
    } 
    else {
        fprintf(stderr, "Failed to parse type\n");
        free(line_copy);
        return PARSE_ERROR;
    }

    /* Extract address */
    token = strtok(NULL, delimiters);
    if (token) {
        strcpy(address, token);
    }
    else { 
        fprintf(stderr, "Failed to parse address\n");
        free(line_copy);
        return PARSE_ERROR;
    }

    /* Extract data (may be optional) */
    token = strtok(NULL, delimiters);
    if (token) {
        strcpy(data, token);
    } 
    else {
        *data = '\0';
    }

    /* Check for anything extra after this */
    token = strtok(NULL, delimiters);
    if (token)
    {
        fprintf(stderr, "Unexpected extra data\n");
        free(line_copy);
        return PARSE_ERROR;
    }

    free(line_copy);    
    
    /* Return pointer to next line, or NULL if this was the last one */
    if (!newline) return NULL;
    return (char*)(newline + 1);
}

/*
   * @brief               Counts the number of memory request lines in the input content. A memory request is defined as a line, a sequence ending with '\n'
   *
   * @param content       Pointer to content to read from
   * 
   * @return              The number of lines found in the content
   * 
*/
unsigned long countRequests(char* content)
{
    /* Use strdup so not to modify the original content */
    size_t requestsCount = 0L;
    char* requests = strdup(content);
    char* ptr = requests;

    /* Count lines */
    while (*ptr){
        if (*ptr == '\n'){
            requestsCount++;
        }
        ptr++;
    }

    /* If the last character wasn't a newline, count the final line */
    if (ptr != content && *(ptr-1) != '\n')
    {
        requestsCount++;
    }

    free(requests);
    return requestsCount;
}

/*
   * @brief               Validates the string value and returns integer
   *
   * @param someValue     String of a value to validate
   * 
   * @return              Validated integer value or parse error
   * 
*/
uint32_t validateValue(char* someValue)
{
    /* Accept both hexadecimal and decimal format */
    char* isEnd;
    long value;
    if (strncmp(someValue, "0x", 2) == 0){
        value = strtol(someValue,&isEnd,16);
    } 
    else {
        value = strtol(someValue,&isEnd,10);
    }
    /* Not a value check */
    if (*isEnd != '\0')
    {
        fprintf(stderr, "Failed to parse value\n");
        return VALUE_ERROR;
    }
    /* Don't accept negative values */
    if (value < 0) {
        fprintf(stderr, "Negative data is not allowed");
        return VALUE_ERROR;
    }

    return (uint32_t)value;
}

/*
   * @brief               Parses a single memory request from raw string inputs and constructs a Request struct
   *
   * @param type          String indicating the request type
   * @param address       String containing the address in hexadecimal or decimal format
   * @param data          String representing the data (should be non-empty only for write requests)
   * @param ok            Pointer to a boolean that will be set to true if the request was valid, false otherwise
   * 
   * @return              A filled Request struct if valid, otherwise a zeroed struct
   * 
*/
struct Request formSingleRequest(char* type, char* address, char* data, bool* ok)
{
    struct Request req = {0};   /* Initialize requests */
    *ok = false;                /*    Assume failure   */
    bool isR = false;

    /* Determine request type */
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

    /* Validate and assing address*/
    uint32_t req_address = validateValue(address);
    if (req_address == VALUE_ERROR){
        printf("Invalid address\n");
        return req;
    } else {
        req.addr = req_address;
    }

    /* Handle data based on request type */
    uint32_t req_data;
    if (isR && *data != '\0'){
        printf("Invalid data --- no data expected by read\n");
        return req;
    } else if (isR) {
        /* Data is empty by R, which is correct */
    } else {

        /* Data empty for W request */
        if (*data == '\0'){
            printf("Missing data for write request\n");
            return req;
        }

        /* Validate value and assgin to Request data*/
        req_data = validateValue(data);

        if (req_data == VALUE_ERROR){
            printf("Invalid data\n");
            return req;
        }
        else {
            req.data = req_data;
        }
    }

    /* If function didn't return earlier, and response is ok */
    *ok = true;
    return req;
}

/*
   * @brief               Receives content and forms requests out of it. If any of them is invalid, returns -1 indicating error
   *
   * @param content       String content to form requests from
   * @param address       String containing the address in hexadecimal or decimal format
   * 
   * @return              Returns either 0 indicating normal procedure or -1 indicating failure
   * 
*/
int formRequests(char* content, struct Request* requests)
{

    /* Allocate memory for type, address and data of a single request */
    char* type    = (char*) calloc(2, sizeof(char));
    char* address = (char*) calloc(21,sizeof(char));
    char* data    = (char*) calloc(21,sizeof(char));

    /* Loop through content */
    size_t request_counter = 0;
    while(content){
        /* Try to store type, address and data from a single line and return pointer to the next line */
        content = split_next_line(content, type, address, data);

        /* If a parse error happes, free resources and return an error */
        if (content == PARSE_ERROR){
            free(type);
            free(address);
            free(data);
            return -1;
        }

        /* Set a response for single request */
        bool ok = false;
        /* Try to form a single request */
        struct Request request = formSingleRequest(type,address,data,&ok);
        /* If response is not okay, free resources and return an error */
        if (!ok) {
            fprintf(stderr,"Failed to form a request\n");
            free(type);
            free(address);
            free(data);
            return -1;
        }

        /* Store single request and increment the pointer */
        requests[request_counter++] = request;
    }

    /* Normal cleanup */
    free(type);
    free(address);
    free(data);

    /* Success */
    return 0;
}