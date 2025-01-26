#include <stdio.h>
#include <string.h>
#include <nvs_flash.h>

#include "dlms.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "mbus.h"
#include "crypto.h"
#include "sdkconfig.h"
#include "wifi.h"
#include "data_sender.h"
#include "uart_handler.h"

#define BYTE_SIZE_UART_DATA 376
#define BYTE_SIZE_PLAINTEXT_BUFFER 1024
#define BYTE_SIZE_SMART_METER_KEY 16

#define READ_MBUS_TASK_STACKSIZE 4096 
#define READ_MBUS_TASK_PRIORITY 3

#define PARSE_DATA_TASK_STACKSIZE 4096
#define PARSE_DATA_TASK_PRIORITY 3

#define SENDER_TASK_STACKSIZE 4096
#define SENDER_TASK_PRIORITY 3

#define MBUS_DATA_QUEUE_LENGTH 10
#define MBUS_DATA_QUEUE_ITEM_SIZE (sizeof(uint8_t) * 376)

#define SENDER_QUEUE_LENGTH 10
#define SENDER_QUEUE_ITEM_SIZE (sizeof(measurement_t))

#define UART_TAG "UART"
#define PARSE_DATA_TAG "PARSE_DATA"
#define SENDER_TAG "SENDER"

TaskHandle_t g_read_mbus_task_handle = NULL;
TaskHandle_t g_parse_data_task_handle = NULL;
TaskHandle_t g_sender_task_handle = NULL;

static QueueHandle_t g_mbus_data_queue_handle = NULL;
static StaticQueue_t g_mbus_data_queue_memory;
static uint8_t g_mbus_data_queue_item_memory[MBUS_DATA_QUEUE_LENGTH * MBUS_DATA_QUEUE_ITEM_SIZE];

static QueueHandle_t g_sender_queue_handle = NULL;
static StaticQueue_t g_sender_queue_memory;
static uint8_t g_sender_queue_item_memory[SENDER_QUEUE_LENGTH * SENDER_QUEUE_ITEM_SIZE];

uint8_t SMART_METER_KEY[BYTE_SIZE_SMART_METER_KEY];

void read_mbus_task_main_func(void* pvParameters) { 
	ESP_LOGI(UART_TAG, "Start reading from mbus");
	uint8_t buffer[BUFFER_SIZE];
	
	while (1) {
        int len = read_uart(buffer, BUFFER_SIZE, pdMS_TO_TICKS(5000));
        
        if (len > 0) {
			bool sent = xQueueSend(g_mbus_data_queue_handle, buffer, 0);
	        if (sent) {
				ESP_LOGI(UART_TAG, "Sent mbus data to queue!");
			} else {
				ESP_LOGE(UART_TAG, "Failed to send mbus data to queue!");
			}
		}
    }
}

void parse_data_task_main_func(void* pvParameters) { 
	uint8_t uart_data[BYTE_SIZE_UART_DATA];
	
	while (1) {	
		if (xQueueReceive(g_mbus_data_queue_handle, uart_data, pdMS_TO_TICKS(5000)) == true) {
		    frame_t frame;
    		parse_buffer_to_frame(&frame, uart_data);
    		
    		uint8_t plaintext[BYTE_SIZE_PLAINTEXT_BUFFER] = {0};
    		size_t plaintext_len = 0;
    		
    		const size_t status_code = decrypt_aes_gcm(SMART_METER_KEY, BYTE_SIZE_KEY, frame.iv, BYTE_SIZE_IV, frame.payload,
                                              BYTE_SIZE_PAYLOAD, plaintext, BYTE_SIZE_PLAINTEXT_BUFFER, &plaintext_len);
                                              
            if (status_code != 0) {
				ESP_LOGE(PARSE_DATA_TAG, "Decryption failed!");
				return;
    		}
    		
    		measurement_t measurement;
    		parse_dlms_payload(&measurement, plaintext, plaintext_len);
    		
    		bool sent = xQueueSend(g_sender_queue_handle, &measurement, 0);
	        if (sent) {
				ESP_LOGI(PARSE_DATA_TAG, "Sent measurement to queue!");
			} else {
				ESP_LOGE(PARSE_DATA_TAG, "Failed to send measurement to queue!");
			}
		}
	}
}

void sender_task_main_func(void* pvParameters) { 
	measurement_t measurement;
	
	while (1) {
		if (xQueueReceive(g_sender_queue_handle, &measurement, pdMS_TO_TICKS(5000)) == true) {
			send_data(&measurement);
		}
	}
}

void parse_smart_meter_key(const char* key_string, uint8_t* key_array) {
    size_t size = 0;

    char* key_copy = strdup(key_string);
    char* token = strtok(key_copy, ",");
    
    while (token != NULL && size < BYTE_SIZE_SMART_METER_KEY) {
        key_array[size++] = (uint8_t)strtol(token, NULL, 0);
        token = strtok(NULL, ",");
    }
    
    free(key_copy);
}

void app_main(void) {
	init_nvs();
	init_wifi();
	vTaskDelay(pdMS_TO_TICKS(5000));
	init_mqtt_client();
	vTaskDelay(pdMS_TO_TICKS(5000));
	init_uart();
	
	const char* smart_meter_key_string = CONFIG_SMART_METER_KEY;
	parse_smart_meter_key(smart_meter_key_string, SMART_METER_KEY);
	
	g_mbus_data_queue_handle = xQueueCreateStatic(MBUS_DATA_QUEUE_LENGTH, MBUS_DATA_QUEUE_ITEM_SIZE, g_mbus_data_queue_item_memory, &g_mbus_data_queue_memory);
	assert(g_mbus_data_queue_handle != NULL);
	
	g_sender_queue_handle = xQueueCreateStatic(SENDER_QUEUE_LENGTH, SENDER_QUEUE_ITEM_SIZE, g_sender_queue_item_memory, &g_sender_queue_memory);
	assert(g_sender_queue_handle != NULL);
	
	xTaskCreate(read_mbus_task_main_func, "READ_MBUS_TASK", READ_MBUS_TASK_STACKSIZE, NULL, READ_MBUS_TASK_PRIORITY, &g_read_mbus_task_handle);
	assert(g_read_mbus_task_handle != NULL);
	
	xTaskCreate(parse_data_task_main_func, "PARSE_DATA_TASK", PARSE_DATA_TASK_STACKSIZE, NULL, PARSE_DATA_TASK_PRIORITY, &g_parse_data_task_handle);
	assert(parse_data_task_main_func != NULL);
	
	xTaskCreate(sender_task_main_func, "SENDER_TASK", SENDER_TASK_STACKSIZE, NULL, SENDER_TASK_PRIORITY, &g_sender_task_handle);
	assert(sender_task_main_func != NULL);
}