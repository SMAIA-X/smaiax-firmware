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

TaskHandle_t gReadMbusTaskHandle = NULL;
TaskHandle_t gParseDataTaskHandle = NULL;
TaskHandle_t gSenderTaskHandle = NULL;

static QueueHandle_t gMbusDataQueueHandle = NULL;
static StaticQueue_t gMbusDataQueueMemory;
static uint8_t gMbusDataQueueItemMemory[MBUS_DATA_QUEUE_LENGTH * MBUS_DATA_QUEUE_ITEM_SIZE];

static QueueHandle_t gSenderQueueHandle = NULL;
static StaticQueue_t gSenderQueueMemory;
static uint8_t gSenderQueueItemMemory[SENDER_QUEUE_LENGTH * SENDER_QUEUE_ITEM_SIZE];

uint8_t SMART_METER_KEY[BYTE_SIZE_SMART_METER_KEY];

void readMbusTaskMainFunc(void* pvParameters) { 
	ESP_LOGI(UART_TAG, "Start reading from mbus");
	uint8_t buffer[BUFFER_SIZE];
	
	while (1) {
        int len = read_uart(buffer, BUFFER_SIZE, pdMS_TO_TICKS(5000));
        
        if (len > 0) {
			bool sent = xQueueSend(gMbusDataQueueHandle, buffer, 0);
	        if (sent) {
				ESP_LOGI(UART_TAG, "Sent mbus data to queue!");
			} else {
				ESP_LOGE(UART_TAG, "Failed to send mbus data to queue!");
			}
		}
    }
}

void parseDataTaskMainFunc(void* pvParameters) { 
	uint8_t uartData[BYTE_SIZE_UART_DATA];
	
	while (1) {	
		if (xQueueReceive(gMbusDataQueueHandle, uartData, 500 / portTICK_PERIOD_MS) == true) {
		    frame_t frame;
    		parseBufferToFrame(&frame, uartData);
    		
    		uint8_t plaintext[BYTE_SIZE_PLAINTEXT_BUFFER] = {0};
    		size_t plaintextLen = 0;
    		
    		const size_t resultCode = decrypt_aes_gcm(SMART_METER_KEY, BYTE_SIZE_KEY, frame.iv, BYTE_SIZE_IV, frame.payload,
                                              BYTE_SIZE_PAYLOAD, plaintext, BYTE_SIZE_PLAINTEXT_BUFFER, &plaintextLen);
                                              
            if (resultCode != 0) {
				ESP_LOGE(PARSE_DATA_TAG, "Decryption failed!");
				return;
    		}
    		
    		measurement_t measurement;
    		parse_dlms_payload(&measurement, plaintext, plaintextLen);
    		
    		bool sent = xQueueSend(gSenderQueueHandle, &measurement, 0);
	        if (sent) {
				ESP_LOGI(PARSE_DATA_TAG, "Sent measurement to queue!");
			} else {
				ESP_LOGE(PARSE_DATA_TAG, "Failed to send measurement to queue!");
			}
		}

		vTaskDelay(pdMS_TO_TICKS(5000));
	}
}

void senderTaskMainFunc(void* pvParameters) { 
	measurement_t measurement;
	
	while (1) {
		if (xQueueReceive(gSenderQueueHandle, &measurement, 500 / portTICK_PERIOD_MS) == true) {
			sendData(&measurement);
		}
	}
}

void parseSmartMeterKey(const char* keyString, uint8_t* keyArray) {
    size_t size = 0;

    char* keyCopy = strdup(keyString);
    char* token = strtok(keyCopy, ",");
    
    while (token != NULL && size < BYTE_SIZE_SMART_METER_KEY) {
        keyArray[size++] = (uint8_t)strtol(token, NULL, 0);
        token = strtok(NULL, ",");
    }
    
    free(keyCopy);
}

void app_main(void) {
	initNvs();
	initWifi();
	vTaskDelay(pdMS_TO_TICKS(5000));
	initMqttClient();
	vTaskDelay(pdMS_TO_TICKS(5000));
	init_uart();
	
	const char* smartMeterKeyString = CONFIG_SMART_METER_KEY;
	parseSmartMeterKey(smartMeterKeyString, SMART_METER_KEY);
	
	gMbusDataQueueHandle = xQueueCreateStatic(MBUS_DATA_QUEUE_LENGTH, MBUS_DATA_QUEUE_ITEM_SIZE, gMbusDataQueueItemMemory, &gMbusDataQueueMemory);
	assert(gMbusDataQueueHandle != NULL);
	
	gSenderQueueHandle = xQueueCreateStatic(SENDER_QUEUE_LENGTH, SENDER_QUEUE_ITEM_SIZE, gSenderQueueItemMemory, &gSenderQueueMemory);
	assert(gSenderQueueHandle != NULL);
	
	xTaskCreate(readMbusTaskMainFunc, "READ_MBUS_TASK", READ_MBUS_TASK_STACKSIZE, NULL, READ_MBUS_TASK_PRIORITY, &gReadMbusTaskHandle);
	assert(gReadMbusTaskHandle != NULL);
	
	xTaskCreate(parseDataTaskMainFunc, "PARSE_DATA_TASK", PARSE_DATA_TASK_STACKSIZE, NULL, PARSE_DATA_TASK_PRIORITY, &gParseDataTaskHandle);
	assert(parseDataTaskMainFunc != NULL);
	
	xTaskCreate(senderTaskMainFunc, "SENDER_TASK", SENDER_TASK_STACKSIZE, NULL, SENDER_TASK_PRIORITY, &gSenderTaskHandle);
	assert(senderTaskMainFunc != NULL);
}