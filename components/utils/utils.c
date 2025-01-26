/*
 * utils.c
 *
 *  Created on: 26.01.2025
 *      Author: michael
 */

#include "utils.h"
#include <string.h>

void parse_smart_meter_key(const char* key_string, uint8_t* key_array, size_t key_size) {
    size_t size = 0;

    char* key_copy = strdup(key_string);
    char* token = strtok(key_copy, ",");
    
    while (token != NULL && size < key_size) {
        key_array[size++] = (uint8_t)strtol(token, NULL, 0);
        token = strtok(NULL, ",");
    }
    
    free(key_copy);
}