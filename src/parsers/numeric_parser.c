#include "../../include/parsers/numeric_parser.h"

/* 
   * @brief         Powerful and unified function to parse a numeric value 
   *
   * @param arg     The string from CLI that needs to be parsed
   * @param out8    Pointer to 8-bit unsigned integer, where validated arg should be saved. If pointer is NULL, then argument should be saved as 32-bit unsigned integer.
   * @param out32   Pointer to 32-bit unsigned integer, where validated arg should be saved, If pointer is NULL, then argument should be saved as 8-bit unsigned integer.
   * @param name    The string containing supposed argument name. If it fails to be validated, the corresponding name will be given to the console
   * 
   * @return        - true if parsing succeeded
   *                - false if parsing failed (out of range, invalid charachters, etc.). Programm should immidiately stop.
   * 
   * @details       The idea of this function is to provide a unified and simplified interface for CLI parsing, without explicit
   *                need to divide a function for different integer sizes. It also consolidates all of the CLI options and 
   *                provides a specific error message regarding one of the input arguments in case. 
   *                
   * @attention     The intended use of this function is by passing either out8 or out32 as NULL, indicating that other pointer
   *                is the one, where argument should be saved. When passing those parametrs as not intended (both are NULL or both aren't)
   *                then it will simply return false, however you should not do as not intended.         
*/
bool parse_unsigned_int(const char* arg, uint8_t* out8, uint32_t* out32, const char* name) {

    /* Not intended use of this function */
    if ((out8 && out32) || (!out8 && !out32)) {
        return false;
    }

    char* endPtr = NULL;
    errno = 0;
    unsigned long value = strtoul(arg, &endPtr, 10);

    /* Verify for basic cases*/
    if (arg[0] == '-') {
        /* We accept only positive values */
        fprintf(stderr, "Negative values are not accepted %s: %s\n", name, arg);
        return false;
    } 
        
    if (*endPtr != '\0') {
        /* Error during parsing */
        fprintf(stderr, "Literals are not accepted in an argument %s: %s\n", name, arg);
        return false;
    } 
    if (errno == ERANGE){
        /* The value is either too small or too big */
        fprintf(stderr, "Buffer error %s: %s\n", name, arg);
        return false;
    }
    
    /* If out32 pointer is not NULL, then try to validate it */
    if (out32) {

        if  (value == 0 || value > UINT32_MAX) {
            fprintf(stderr, "Out of bounds %s: %s\n", name, arg);
            return false;
        }

        /* Validation succeeded, store the value at given pointer */
        *out32 = (uint32_t) value;

    } 
    /* If out8 pointer is not NULL, then save it */
    else {
        /* Store the value at given pointer */
        *out8 = (uint8_t) value;
    }

    /* Verification is successful. Return true as an indicator of success */
    return true;
}