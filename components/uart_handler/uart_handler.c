/*
 * uart_handler.c
 *
 *  Created on: 23.01.2025
 *      Author: michael
 */

#include "uart_handler.h"
#include "driver/uart.h"
#include "driver/gpio.h"

void init_uart(void) {
    const uart_config_t uart_config = {
        .baud_rate = BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = 4,
    };

    ESP_ERROR_CHECK(uart_driver_install(UART_NUM, BUFFER_SIZE * 2, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(UART_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM, TXD_PIN, RXD_PIN, RTS, CTS));
}

int read_uart(uint8_t* buffer, size_t max_len, TickType_t tick_to_wait) {
    int len = uart_read_bytes(UART_NUM, buffer, max_len, tick_to_wait);
    
    return len;
}