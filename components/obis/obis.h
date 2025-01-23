/*
 * obis.h
 *
 *  Created on: 19.01.2025
 *      Author: michael
 */

#ifndef COMPONENTS_OBIS_OBIS_H_
#define COMPONENTS_OBIS_OBIS_H_

#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint16_t year; 
    uint8_t month; 
    uint8_t day; 
    uint8_t hour; 
    uint8_t minute; 
    uint8_t second;
    uint32_t id;
    float voltage_phase_1;
    float voltage_phase_2;
    float voltage_phase_3;
    float current_phase_1;
    float current_phase_2;
    float current_phase_3;
    float positive_active_power;
    float negative_active_power;
    float reactive_power_plus;
    float reactive_power_minus;
    float positive_active_energy_total;
    float negative_active_energy_total;
} measurement_t;

void parse_obis_codes(measurement_t* measurement, uint8_t* data, size_t data_len);

#endif /* COMPONENTS_OBIS_OBIS_H_ */
