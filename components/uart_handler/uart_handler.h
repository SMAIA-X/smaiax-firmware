/*
 * uart_handler.h
 *
 *  Created on: 23.01.2025
 *      Author: michael
 */

#ifndef COMPONENTS_UART_HANDLER_UART_HANDLER_H_
#define COMPONENTS_UART_HANDLER_UART_HANDLER_H_

#include <stdint.h>
#include <stddef.h>
#include "freertos/portmacro.h"

#define TXD_PIN GPIO_NUM_10
#define RXD_PIN GPIO_NUM_11
#define RTS UART_PIN_NO_CHANGE
#define CTS UART_PIN_NO_CHANGE

#define UART_NUM UART_NUM_1
#define BAUD_RATE 2400

#define UART_TAG "UART"

static const int BUFFER_SIZE = 1024;

void init_uart();
int read_uart(uint8_t* buffer, size_t max_len, TickType_t tick_to_wait);

#endif /* COMPONENTS_UART_HANDLER_UART_HANDLER_H_ */
