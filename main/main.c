#include <stdio.h>
#include <string.h>
#include <nvs_flash.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "mbus.h"
#include "crypto.h"
#include "sdkconfig.h"
#include "wifi.h"
#include "obis.h"
#include "mqtt_client.h"

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

esp_mqtt_client_handle_t client;

uint8_t SMART_METER_KEY[BYTE_SIZE_SMART_METER_KEY];

void readMbusTaskMainFunc(void* pvParameters) { 
	// TODO: Replace with actual uart reading code
	ESP_LOGI(UART_TAG, "Start reading from mbus");
	uint8_t uartData[BYTE_SIZE_UART_DATA] = {
	0x68, 0xFA, 0xFA, 0x68, 0x53, 0xFF, 0x00, 0x01, 0x67, 0xDB, 0x08, 0x4B, 0x46, 0x4D, 0x10, 0x20, 0x03, 0x1D, 0x00, 0x82, 0x01,
	0x55, 0x21, 0x00, 0x02, 0x77, 0x99, 0x5E, 0xF9, 0xEE, 0x29, 0x0D, 0xED, 0x5D, 0x9F, 0x1F, 0xBE, 0x87, 0x30, 0x90, 0x46, 0x50, 
	0xDE, 0x64, 0x7A, 0x44, 0x09, 0xE4, 0x73, 0x5D, 0xD1, 0x05, 0x78, 0x52, 0xC9, 0x21, 0x17, 0x30, 0x29, 0x98, 0x7C, 0x1C, 0xE6, 
	0x7C, 0x11, 0xE7, 0x24, 0x1C, 0xC3, 0xCA, 0x31, 0x7B, 0xC2, 0xF5, 0x96, 0x60, 0x45, 0x49, 0x0D, 0xD2, 0xCB, 0x15, 0x3D, 0x66, 
	0x36, 0x5F, 0x06, 0x49, 0xED, 0x55, 0xB7, 0xA4, 0x70, 0x37, 0x34, 0x60, 0xA9, 0xBC, 0xA6, 0xFE, 0x2F, 0xFF, 0x5F, 0x9B, 0xE8, 
	0xDB, 0xDA, 0xA1, 0x4C, 0xCA, 0x4B, 0xC7, 0xCA, 0xB1, 0xCE, 0xE4, 0xF2, 0x4E, 0x60, 0xB1, 0xA4, 0x72, 0xD9, 0x9D, 0x6F, 0xBD, 
	0x12, 0xDE, 0x61, 0x2D, 0xBE, 0x2C, 0xB4, 0x59, 0x03, 0x7C, 0xAB, 0x6C, 0x39, 0x8D, 0x10, 0x5C, 0xA3, 0x25, 0xDD, 0x9C, 0x70, 
	0xEE, 0xED, 0xD8, 0x56, 0x1B, 0xF0, 0x7A, 0x2E, 0x6A, 0xFA, 0x27, 0xF8, 0x21, 0x57, 0x66, 0x69, 0x66, 0x89, 0xB2, 0xE1, 0x8E, 
	0xF5, 0x19, 0x3D, 0x82, 0xD6, 0xBF, 0x24, 0xE1, 0x38, 0x20, 0x3B, 0x6C, 0xC7, 0xC4, 0x55, 0x15, 0x87, 0x39, 0x75, 0x23, 0x85, 
	0x19, 0x2A, 0xE4, 0xE0, 0x15, 0x5F, 0xB2, 0x3E, 0x7C, 0xEB, 0x3A, 0xF8, 0xF3, 0xCE, 0x0F, 0x04, 0xF2, 0x1D, 0xE2, 0x1A, 0x1F, 
	0xF7, 0x71, 0xE3, 0xCD, 0x17, 0xCC, 0xEB, 0xF0, 0x13, 0xA3, 0x84, 0x1F, 0xD2, 0xC7, 0x64, 0x46, 0x46, 0xD8, 0x27, 0xB9, 0x92, 
	0x99, 0x83, 0x2B, 0xB6, 0x83, 0x97, 0xF2, 0xC8, 0xB4, 0x03, 0x26, 0x53, 0x3F, 0x3B, 0x8E, 0x60, 0xDC, 0x46, 0x30, 0x5F, 0xDF, 
	0x62, 0xA4, 0x93, 0x16, 0x68, 0x72, 0x72, 0x68, 0x53, 0xFF, 0x11, 0x01, 0x67, 0xB5, 0xA5, 0x87, 0xF5, 0xC4, 0xC8, 0x1A, 0x1A, 
	0x54, 0xBC, 0x6E, 0x2C, 0xFF, 0x7A, 0x3F, 0x7A, 0xC6, 0xF5, 0x82, 0x8E, 0x8B, 0x60, 0xD0, 0xB5, 0x9E, 0xCF, 0x2A, 0xE4, 0x6F, 
	0x1F, 0x4D, 0x2B, 0x6C, 0xE0, 0xB4, 0x9C, 0x01, 0x73, 0x5D, 0x0D, 0xD7, 0x94, 0xF8, 0x3D, 0xEA, 0x5C, 0x31, 0x75, 0x7D, 0xC3, 
	0x63, 0xF8, 0x1C, 0x76, 0x80, 0xA8, 0xAE, 0x5A, 0xEE, 0xE1, 0x7F, 0x1D, 0x08, 0xDA, 0xB1, 0xBF, 0xA9, 0x78, 0x9B, 0x87, 0x7D, 
	0x16, 0xA4, 0x86, 0x2F, 0x04, 0x50, 0x11, 0x59, 0xE6, 0x4D, 0xDD, 0x74, 0x6E, 0xC4, 0xAC, 0x14, 0xC1, 0x53, 0x92, 0xAE, 0xF2, 
	0x4D, 0x5F, 0x54, 0xC5, 0x63, 0xDC, 0x1A, 0xDA, 0x39, 0x62, 0x02, 0xD8, 0xDF, 0xB9, 0x8B, 0xE9, 0xDA, 0x15, 0x16
	};
	
	while (1) {
        ESP_LOGI(UART_TAG, "Reading from mbus...");
        
        bool sent = xQueueSend(gMbusDataQueueHandle, uartData, 0);
        if (sent) {
			ESP_LOGI(UART_TAG, "Sent mbus data to queue!");
		} else {
			ESP_LOGE(UART_TAG, "Failed to send mbus data to queue!");
		}
        
        vTaskDelay(pdMS_TO_TICKS(5000));
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
    		parse_obis_codes(&measurement, plaintext, plaintextLen);
    		
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

// TODO: Move to own component
static void log_error_if_nonzero(const char *message, int error_code) {
    if (error_code != 0) {
        ESP_LOGE(SENDER_TAG, "Last error %s: 0x%x", message, error_code);
    }
}

// TODO: Move to own component
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    ESP_LOGD(SENDER_TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    client = event->client;
    
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(SENDER_TAG, "MQTT_EVENT_CONNECTED");
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(SENDER_TAG, "MQTT_EVENT_DISCONNECTED");
        break;
    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(SENDER_TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(SENDER_TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(SENDER_TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
     case MQTT_EVENT_DATA:
        ESP_LOGI(SENDER_TAG, "MQTT_EVENT_DATA");
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(SENDER_TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
			log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(SENDER_TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
        }
        break;
    default:
        ESP_LOGI(SENDER_TAG, "Other event id:%d", event->event_id);
        break;
    }
}

// TODO: Move to own component
void initMqttClient() {
	esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = CONFIG_MQTT_BROKER_URL,
        .credentials.username = CONFIG_MQTT_USERNAME,
        .credentials.authentication.password = CONFIG_MQTT_PASSWORD
    };
    
    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}

void senderTaskMainFunc(void* pvParameters) { 
	measurement_t measurement;
	int msg_id;
	
	while (1) {
		if (xQueueReceive(gSenderQueueHandle, &measurement, 500 / portTICK_PERIOD_MS) == true) {
			msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "{\n\"data\": \"data\"\n}", 0, 0, 0);
			
			if (msg_id != 0) {
				ESP_LOGE(SENDER_TAG, "Publish failed, msg_id=%d", msg_id);
			} else {
				ESP_LOGI(SENDER_TAG, "Publish successful, msg_id=%d", msg_id);	
			}
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