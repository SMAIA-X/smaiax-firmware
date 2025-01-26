/*
 * mbus.c
 *
 *  Created on: 18.01.2025
 *      Author: michael
 */


#include "mbus.h"

void parse_buffer_to_frame(frame_t* frame, const uint8_t* buffer) {
    uint32_t iv_index = 0, payload_index = 0, buffer_index;

    for (buffer_index = SYSTEM_TITLE_START_BIT; iv_index < BYTE_SIZE_SYSTEM_TITLE; iv_index++, buffer_index++) {
        frame->iv[iv_index] = buffer[buffer_index];
    }

    for (buffer_index = FRAME_COUNTER_START_BIT; iv_index < BYTE_SIZE_IV; iv_index++, buffer_index++) {
        frame->iv[iv_index] = buffer[buffer_index];
    }

    for (buffer_index = PAYLOAD_FIRST_FRAME_START_BIT; payload_index < BYTE_SIZE_PAYLOAD_FIRST_FRAME; payload_index++, buffer_index++) {
        frame->payload[payload_index] = buffer[buffer_index];
    }

    for (buffer_index = PAYLOAD_SECOND_FRAME_START_BIT; payload_index < BYTE_SIZE_PAYLOAD; payload_index++, buffer_index++) {
        frame->payload[payload_index] = buffer[buffer_index];
    }
}