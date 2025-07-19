#include "../../include/parsers/numeric_parser.h"

/* 
   * @brief         Function to validate a decimal string value
   *
   * @param arg     The string from CLI that needs to be parsed
   * @param name    The string containing supposed argument name. If it fails to be validated, the corresponding name will be given to the console
   * 
   * @return        - parsed value or an INVALID_VALUE macro, if verification fails.
   *              
*/
unsigned long validate_value_decimal(const char* arg, const char* name)
{
    char* endPtr = NULL;
    errno = 0;
    unsigned long value = strtoul(arg, &endPtr, 10);

    /* Verify for basic cases*/
    if (arg[0] == '-') {
        /* We accept only positive values */
        fprintf(stderr, "Negative values are not accepted %s: %s\n", name, arg);
        return INVALID_VALUE;
    } 
        
    if (*endPtr != '\0') {
        /* Error during parsing */
        fprintf(stderr, "Literals are not accepted in an argument %s: %s\n", name, arg);
        return INVALID_VALUE;
    } 
    if (errno == ERANGE){
        /* The value is either too small or too big */
        fprintf(stderr, "Buffer error %s: %s\n", name, arg);
        return INVALID_VALUE;
    }

    return value;
}

/* 
   * @brief         Function to parse and write a value into 32 bit unsigned integer
   *
   * @param arg     The string from CLI that needs to be parsed
   * @param out32   Pointer to 32-bit unsigned integer, where validated arg should be saved.
   * @param name    The string containing supposed argument name. If it fails to be validated, the corresponding name will be given to the console
   * 
   * @return        - true if parsing succeeded
   *                - false if parsing failed (out of range, invalid charachters, etc.). Programm should immidiately stop.
   *    
*/
bool parse_unsigned_int32(const char* arg, uint32_t* out32, const char* name) {

    unsigned long value;

    /* Validate value */
    if ((value = validate_value_decimal(arg, name)) == INVALID_VALUE) {
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

    /* Verification is successful. Return true as an indicator of success */
    return true;
}

/* 
   * @brief         Function to parse and write a value into 8 bit unsigned integer
   *
   * @param arg     The string from CLI that needs to be parsed
   * @param out8    Pointer to 8-bit unsigned integer, where validated arg should be saved
   * @param name    The string containing supposed argument name. If it fails to be validated, the corresponding name will be given to the console
   * 
   * @return        - true if parsing succeeded
   *                - false if parsing failed (out of range, invalid charachters, etc.). Programm should immidiately stop.
   *    
*/
bool parse_unsigned_int8(const char* arg, uint8_t* out8, const char* name) {

    unsigned long value;

    /* Validate value */
    if ((value = validate_value_decimal(arg, name)) == INVALID_VALUE) {
        return false;
    }
    
    /* If out8 pointer is not NULL, then try to validate it */
    if (out8) {

        /* Validation succeeded, store the value at given pointer */
        *out8 = (uint8_t) value;
    } 

    /* Verification is successful. Return true as an indicator of success */
    return true;
}