/*
 * dlms.h
 *
 *  Created on: 19.01.2025
 *      Author: michael
 */

#ifndef COMPONENTS_DLMS_DLMS_H_
#define COMPONENTS_DLMS_DLMS_H_

#include <stdint.h>
#include <stddef.h>

#define BYTE_SIZE_TIMESTAMP 12

typedef struct {
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    double voltage_phase_1;
    double voltage_phase_2;
    double voltage_phase_3;
    double current_phase_1;
    double current_phase_2;
    double current_phase_3;
    double positive_active_power;
    double negative_active_power;
    double positive_active_energy_total;
    double negative_active_energy_total;
    double positive_reactive_energy_total;
    double negative_reactive_energy_total;
} measurement_t;

void parse_dlms_payload(measurement_t* measurement, uint8_t* data, size_t data_len);

#endif /* COMPONENTS_DLMS_DLMS_H_ */
