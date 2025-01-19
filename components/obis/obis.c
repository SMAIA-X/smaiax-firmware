/*
 * obis.c
 *
 *  Created on: 19.01.2025
 *      Author: michael
 *
 * This code is adapted from the original implementation found in:
 * https://github.com/LaendleEnergy/smart-meter-reader/blob/main/components/MBus_SmartMeter/kaifa.c
 */


#include "obis.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

uint8_t* find_in_mem(uint8_t* haystack, uint8_t* needle, size_t haystack_size, size_t needle_size) {
    for (size_t i = 0; i < haystack_size - needle_size; i++) {
        size_t sum = 0;
        for (size_t j = 0; j < needle_size; j++) {
            sum += *(haystack + i + j) == *(needle + j);
        }
        if (sum == needle_size) {
            return haystack + i;
        }
    }

    return NULL;
}

uint32_t get_hash(char* s, size_t n) {
    int64_t p = 31, m = 1e9 + 7;
    int64_t hash = 0;
    int64_t p_pow = 1;
    for(int i = 0; i < n; i++) {
        hash = (hash + s[i] * p_pow) % m;
        p_pow = (p_pow * p) % m;
    }
    return hash;
}

void parse_obis_codes(measurement_t* measurement, uint8_t * data, size_t data_len){
    uint8_t obis_size = 6;
    int8_t scale_voltage = 0;
    int8_t scale_current = 0;
	int8_t scale_power = 0;
	int8_t scale_energy = 0;

    uint8_t obis_timestamp[6] = {0x00, 0x00, 0x01, 0x00, 0x00, 0xff};
    uint8_t * start = find_in_mem(data, obis_timestamp, data_len, obis_size);
    if(start!=NULL){
        start+=obis_size+2;
        measurement->year = *(start) << 8 | *(start+1);
        measurement->month = *(start+2);
        measurement->day = *(start+3);
        measurement->hour = *(start+5);
        measurement->minute = *(start+6);
        measurement->second = *(start+7);
    }

    uint8_t obis_meternumber[6] = {0x00, 0x00, 0x60, 0x01, 0x00, 0xff};
    start = find_in_mem(data, obis_meternumber, data_len, obis_size);
    if(start!=NULL){
        start+=obis_size+1;
        uint8_t meternumber_length = *(start);
        char* meternumber = malloc(sizeof(char) * meternumber_length);
        memcpy(meternumber, start+1, meternumber_length);

        measurement->id = get_hash(meternumber, meternumber_length);
        free(meternumber);
    }

    uint8_t obis_voltage_l1[6] = {0x01, 0x00, 0x20, 0x07, 0x00, 0xff};
    start = find_in_mem(data, obis_voltage_l1, data_len, obis_size);
    if(start!=NULL){
        start+=obis_size+1;
        uint16_t voltage_l1 = *(start)<< 8 | *(start+1);
        int8_t scale_factor = *(start+5);

        scale_voltage = scale_factor;
        measurement->voltage_l1 = voltage_l1 * powf(10, scale_voltage);
    }

    uint8_t obis_voltage_l2[6] = {0x01, 0x00, 0x34, 0x07, 0x00, 0xff};
    start = find_in_mem(data, obis_voltage_l2, data_len, obis_size);
    if(start!=NULL){
        start+=obis_size+1;
        uint16_t voltage_l2 = *(start)<< 8 | *(start+1);

        measurement->voltage_l2 = voltage_l2 * powf(10, scale_voltage);
    }

    uint8_t obis_voltage_l3[6] = {0x01, 0x00, 0x48, 0x07, 0x00, 0xff};
    start = find_in_mem(data, obis_voltage_l3, data_len, obis_size);
    if(start!=NULL){
        start+=obis_size+1;
        uint16_t voltage_l3 = *(start)<< 8 | *(start+1);

        measurement->voltage_l3 = voltage_l3 * powf(10, scale_voltage);
    }

    uint8_t obis_current_l1[6] = {0x01, 0x00, 0x1F, 0x07, 0x00, 0xff};
    start = find_in_mem(data, obis_current_l1, data_len, obis_size);
    if(start!=NULL){
        start+=obis_size+1;
        uint16_t current_l1_buf = *(start)<< 8 | *(start+1);
        int8_t scale_factor = *(start+5);

        scale_current = scale_factor;
        measurement->current_l1 = current_l1_buf * powf(10, scale_current);
    }

    uint8_t obis_current_l2[6] = {0x01, 0x00, 0x33, 0x07, 0x00, 0xff};
    start = find_in_mem(data, obis_current_l2, data_len, obis_size);
    if(start!=NULL){
        start+=obis_size+1;
        uint16_t current_l2_buf = *(start)<< 8 | *(start+1);

        measurement->current_l2 = current_l2_buf * powf(10, scale_current);
    }

    uint8_t obis_current_l3[6] = {0x01, 0x00, 0x47, 0x07, 0x00, 0xff};
    start = find_in_mem(data, obis_current_l3, data_len, obis_size);
    if(start!=NULL){
        start+=obis_size+1;
        uint16_t current_l3_buf = *(start)<< 8 | *(start+1);

        measurement->current_l3 = current_l3_buf * powf(10, scale_current);
    }

    uint8_t obis_active_power_plus[6] = {0x01, 0x00, 0x01, 0x07, 0x00, 0xff};
    start = find_in_mem(data, obis_active_power_plus, data_len, obis_size);
    if(start!=NULL){
        start+=obis_size+1;
        uint32_t active_power_plus_buf = *(start)<< 24 | *(start+1)<<16 | *(start+2)<< 8 | *(start+3);
        int8_t scale_factor = *(start+7);

		scale_power = scale_factor;
        measurement->active_power_plus = active_power_plus_buf * powf(10, scale_power);
    }

    uint8_t obis_active_power_minus[6] = {0x01, 0x00, 0x02, 0x07, 0x00, 0xff};
    start = find_in_mem(data, obis_active_power_minus, data_len, obis_size);
    if(start!=NULL){
        start+=obis_size+1;
        uint32_t active_power_minus_buf = *(start)<< 24 | *(start+1)<<16 | *(start+2)<< 8 | *(start+3);

        measurement->active_power_minus = active_power_minus_buf * powf(10, scale_power);
    }

    uint8_t obis_active_energy_plus[6] = {0x01, 0x00, 0x01, 0x08, 0x00, 0xff};
    start = find_in_mem(data, obis_active_energy_plus, data_len, obis_size);
    if(start!=NULL){
        start+=obis_size+1;
        uint32_t active_energy_plus_buf = *(start)<< 24 | *(start+1)<<16 | *(start+2)<< 8 | *(start+3);
        int8_t scale_factor = *(start+7);

		scale_energy = scale_factor;
        measurement->active_energy_plus = active_energy_plus_buf * powf(10, scale_energy);
    }

    uint8_t obis_active_energy_minus[6] = {0x01, 0x00, 0x02, 0x08, 0x00, 0xff};
    start = find_in_mem(data, obis_active_energy_minus, data_len, obis_size);
    if(start!=NULL){
        start+=obis_size+1;
        uint32_t active_energy_minus_buf = *(start)<< 24 | *(start+1)<<16 | *(start+2)<< 8 | *(start+3);

        measurement->active_energy_minus = active_energy_minus_buf * powf(10, scale_energy);
    }

    uint8_t obis_reactive_power_plus[6] = {0x01, 0x00, 0x01, 0x07, 0x00, 0xff};
    start = find_in_mem(data, obis_reactive_power_plus, data_len, obis_size);
    if(start!=NULL){
        start+=obis_size+1;
        uint16_t reactive_power_plus_buf = *(start)<< 24 | *(start+1)<<16 | *(start+2)<< 8 | *(start+3);

        measurement->reactive_power_plus = reactive_power_plus_buf * powf(10, scale_power);
    }

    uint8_t obis_reactive_power_minus[6] = {0x01, 0x00, 0x02, 0x07, 0x00, 0xff};
    start = find_in_mem(data, obis_reactive_power_minus, data_len, obis_size);
    if(start!=NULL){
        start+=obis_size+1;
        uint16_t reactive_power_minus_buf = *(start)<< 24 | *(start+1)<<16 | *(start+2)<< 8 | *(start+3);

        measurement->reactive_power_minus = reactive_power_minus_buf * powf(10, scale_power);
    }
}