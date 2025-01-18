/*
 * mbus.h
 *
 *  Created on: 18.01.2025
 *      Author: michael
 */

#ifndef COMPONENTS_MBUS_H_
#define COMPONENTS_MBUS_H_

#include <stdint.h>

#define BYTE_SIZE_SYSTEM_TITLE 8
#define BYTE_SIZE_IV 12
#define BYTE_SIZE_PAYLOAD 336
#define BYTE_SIZE_PAYLOAD_FIRST_FRAME 227
#define BYTE_SIZE_PAYLOAD_SECOND_FRAME 109
#define SYSTEM_TITLE_START_BIT 11
#define FRAME_COUNTER_START_BIT 23
#define PAYLOAD_FIRST_FRAME_START_BIT 27
#define PAYLOAD_SECOND_FRAME_START_BIT 265

typedef struct {
    uint8_t iv[BYTE_SIZE_IV];
    uint8_t payload[BYTE_SIZE_PAYLOAD];
} frame_t;

/**
 * Parses an M-Bus frame from the given buffer.
 *
 * @param frame Pointer to the frame structure to be filled.
 * @param buffer Pointer to the buffer containing the M-Bus frame data.
 */
void parseBufferToFrame(frame_t* frame, const uint8_t* buffer);

#endif /* COMPONENTS_MBUS_H_ */
