/*
 * mbus.c
 *
 *  Created on: 18.01.2025
 *      Author: michael
 */


#include "mbus.h"

void parseBufferToFrame(frame_t* frame, const uint8_t* buffer) {
    uint32_t ivIndex = 0, payloadIndex = 0, bufferIndex;

    for (bufferIndex = SYSTEM_TITLE_START_BIT; ivIndex < BYTE_SIZE_SYSTEM_TITLE; ivIndex++, bufferIndex++) {
        frame->iv[ivIndex] = buffer[bufferIndex];
    }

    for (bufferIndex = FRAME_COUNTER_START_BIT; ivIndex < BYTE_SIZE_IV; ivIndex++, bufferIndex++) {
        frame->iv[ivIndex] = buffer[bufferIndex];
    }

    for (bufferIndex = PAYLOAD_FIRST_FRAME_START_BIT; payloadIndex < BYTE_SIZE_PAYLOAD_FIRST_FRAME; payloadIndex++, bufferIndex++) {
        frame->payload[payloadIndex] = buffer[bufferIndex];
    }

    for (bufferIndex = PAYLOAD_SECOND_FRAME_START_BIT; payloadIndex < BYTE_SIZE_PAYLOAD; payloadIndex++, bufferIndex++) {
        frame->payload[payloadIndex] = buffer[bufferIndex];
    }
}