#include <stdio.h>
#include <string.h>
#include <nvs_flash.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "mbus.h"
#include "crypto.h"
#include "sdkconfig.h"
#include "esp_wifi.h"
#include "esp_err.h"
#include "esp_netif.h"

#define BYTE_SIZE_UART_DATA 376
#define BYTE_SIZE_PLAINTEXT_BUFFER 1024
#define BYTE_SIZE_SMART_METER_KEY 16

#define READ_MBUS_TASK_STACKSIZE 4096 
#define READ_MBUS_TASK_PRIORITY 3

#define PARSE_DATA_TASK_STACKSIZE 4096
#define PARSE_DATA_TASK_PRIORITY 3

#define MBUS_DATA_QUEUE_LENGTH 10
#define MBUS_DATA_QUEUE_ITEM_SIZE (sizeof(uint8_t) * 376)

#define UART_TAG "UART"
#define PARSE_DATA_TAG "PARSE_DATA"
#define WIFI_TAG "WIFI"

TaskHandle_t gReadMbusTaskHandle = NULL;
TaskHandle_t gParseDataTaskHandle = NULL;
static QueueHandle_t gMbusDataQueueHandle = NULL;
static StaticQueue_t gMbusDataQueueMemory;
static uint8_t gMbusDataQueueItemMemory[MBUS_DATA_QUEUE_LENGTH * MBUS_DATA_QUEUE_ITEM_SIZE];

static esp_ip4_addr_t gIPAddr;

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
	while (1) {
		uint8_t uartData[BYTE_SIZE_UART_DATA];
		
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
		}

		vTaskDelay(pdMS_TO_TICKS(5000));
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

void on_wifi_disconnect(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    ESP_LOGE(WIFI_TAG, "Wi-Fi disconnected, trying to reconnect...");
    esp_err_t err = esp_wifi_connect();
    if (err == ESP_ERR_WIFI_NOT_STARTED) {
        return;
    }
    ESP_ERROR_CHECK(err);
}

static void on_got_ip(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    ESP_LOGI(WIFI_TAG, "Got IPv4 event: Interface \"%s\" address: " IPSTR, esp_netif_get_desc(event->esp_netif), IP2STR(&event->ip_info.ip));
    memcpy(&gIPAddr, &event->ip_info.ip, sizeof(gIPAddr));
}

void initNvs() {
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase()); // NVS partition was truncated and needs to be erased
		ESP_ERROR_CHECK(nvs_flash_init()); // Retry nvs_flash_init
	}
	ESP_ERROR_CHECK(ret);
}

void initWifi() {
	ESP_ERROR_CHECK(esp_netif_init());
	ESP_ERROR_CHECK(esp_event_loop_create_default());

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));
	esp_netif_inherent_config_t esp_netif_config = ESP_NETIF_INHERENT_DEFAULT_WIFI_STA();
	esp_netif_create_wifi(WIFI_IF_STA, &esp_netif_config);
	esp_wifi_set_default_wifi_sta_handlers();
	ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &on_wifi_disconnect, NULL));
	ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &on_got_ip, NULL));

	ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
	wifi_config_t wifi_config = {
		.sta = {
			.ssid = CONFIG_WIFI_SSID,
			.password = CONFIG_WIFI_PASSWORD,
			.scan_method = WIFI_FAST_SCAN,
			.sort_method = WIFI_CONNECT_AP_BY_SIGNAL,
			.threshold.rssi = -127,
			.threshold.authmode = WIFI_AUTH_WPA2_PSK,
		},
	};

	 ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	 ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
	 ESP_ERROR_CHECK(esp_wifi_start());
	 esp_wifi_connect();
}

void app_main(void) {
	initNvs();
	initWifi();
	
	const char* smartMeterKeyString = CONFIG_SMART_METER_KEY;
	parseSmartMeterKey(smartMeterKeyString, SMART_METER_KEY);
	
	gMbusDataQueueHandle = xQueueCreateStatic(MBUS_DATA_QUEUE_LENGTH, MBUS_DATA_QUEUE_ITEM_SIZE, gMbusDataQueueItemMemory, &gMbusDataQueueMemory);
	assert(gMbusDataQueueHandle != NULL);
	
	xTaskCreate(readMbusTaskMainFunc, "READ_MBUS_TASK", READ_MBUS_TASK_STACKSIZE, NULL, READ_MBUS_TASK_PRIORITY, &gReadMbusTaskHandle);
	assert(gReadMbusTaskHandle != NULL);
	
	xTaskCreate(parseDataTaskMainFunc, "PARSE_DATA_TASK", PARSE_DATA_TASK_STACKSIZE, NULL, PARSE_DATA_TASK_PRIORITY, &gParseDataTaskHandle);
	assert(gReadMbusTaskHandle != NULL);
}