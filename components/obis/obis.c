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
#include <math.h>

void parse_obis_codes(measurement_t* measurement, uint8_t * data, size_t data_len){
   uint32_t offset = 6; // Skip the first 6 bytes of the data and start with the timestamp
    uint8_t timestamp[BYTE_SIZE_TIMESTAMP];

    for (uint8_t i = 0; i < BYTE_SIZE_TIMESTAMP; i++) {
        timestamp[i] = data[offset];
        offset++;
    }

    measurement->year = (timestamp[0] << 8) | timestamp[1];
    measurement->month = timestamp[2];
    measurement->day = timestamp[3];
    measurement->hour = timestamp[5];
    measurement->minute = timestamp[6];
    measurement->second = timestamp[7];

    offset = 107;
    const uint16_t voltage_phase_1 = (data[offset] << 8) | data[offset + 1];
    const int8_t voltage_phase_1_scale = data[offset + 5];
    measurement->voltage_phase_1 = voltage_phase_1 * pow(10, voltage_phase_1_scale);
    offset = offset + 19;

    const uint16_t voltage_phase_2 = (data[offset] << 8) | data[offset + 1];
    const int8_t voltage_phase_2_scale = data[offset + 5];
    measurement->voltage_phase_2 = voltage_phase_2 * pow(10, voltage_phase_2_scale);
    offset = offset + 19;

    const uint16_t voltage_phase_3 = (data[offset] << 8) | data[offset + 1];
    const int8_t voltage_phase_3_scale = data[offset + 5];
    measurement->voltage_phase_3 = voltage_phase_3 * pow(10, voltage_phase_3_scale);
    offset = offset + 19;

    const uint16_t current_phase_1 = (data[offset] << 8) | data[offset + 1];
    const int8_t current_phase_1_scale = data[offset + 5];
    measurement->current_phase_1 = current_phase_1 * pow(10, current_phase_1_scale);
    offset = offset + 19;

    const uint16_t current_phase_2 = (data[offset] << 8) | data[offset + 1];
    const int8_t current_phase_2_scale = data[offset + 5];
    measurement->current_phase_2 = current_phase_2 * pow(10, current_phase_2_scale);
    offset = offset + 19;

    const uint16_t current_phase_3 = (data[offset] << 8) | data[offset + 1];
    const int8_t current_phase_3_scale = data[offset + 5];
    measurement->current_phase_3 = current_phase_3 * pow(10, current_phase_3_scale);
    offset = offset + 19;

    const uint32_t positive_active_power = (data[offset] << 24) | (data[offset + 1] << 16) | (data[offset + 2] << 8) | data[offset + 3];
    const int8_t positive_active_power_scale = data[offset + 7];
    measurement->positive_active_power = positive_active_power * pow(10, positive_active_power_scale);
    offset = offset + 21;

    const uint32_t negative_active_power = (data[offset] << 24) | (data[offset + 1] << 16) | (data[offset + 2] << 8) | data[offset + 3];
    const int8_t negative_active_power_scale = data[offset + 7];
    measurement->negative_active_power = negative_active_power * pow(10, negative_active_power_scale);
    offset = offset + 21;

    const uint32_t positive_active_energy_total = (data[offset] << 24) | (data[offset + 1] << 16) | (data[offset + 2] << 8) | data[offset + 3];
    const int8_t positive_active_energy_total_scale = data[offset + 7];
    measurement->positive_active_energy_total = positive_active_energy_total * pow(10, positive_active_energy_total_scale);
    offset = offset + 21;

    const uint32_t negative_active_energy_total = (data[offset] << 24) | (data[offset + 1] << 16) | (data[offset + 2] << 8) | data[offset + 3];
    const int8_t negative_active_energy_total_scale = data[offset + 7];
    measurement->negative_active_energy_total = negative_active_energy_total * pow(10, negative_active_energy_total_scale);
    offset = offset + 21;

    const uint32_t positive_reactive_energy_total = (data[offset] << 24) | (data[offset + 1] << 16) | (data[offset + 2] << 8) | data[offset + 3];
    const int8_t positive_reactive_energy_total_scale = data[offset + 7];
    measurement->positive_reactive_energy_total = positive_reactive_energy_total * pow(10, positive_reactive_energy_total_scale);
    offset = offset + 21;

	const uint32_t negative_reactive_energy_total = (data[offset] << 24) | (data[offset + 1] << 16) | (data[offset + 2] << 8) | data[offset + 3];
    const int8_t negative_reactive_energy_total_scale = data[offset + 7];
    measurement->negative_reactive_energy_total = negative_reactive_energy_total * pow(10, negative_reactive_energy_total_scale);
}