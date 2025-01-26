/*
 * wifi.c
 *
 *  Created on: 19.01.2025
 *      Author: michael
 */

#include "wifi.h"
#include <nvs_flash.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_err.h"
#include "esp_netif.h"
#include "esp_log.h"

static esp_ip4_addr_t gIPAddr;

void init_nvs() {
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase()); // NVS partition was truncated and needs to be erased
		ESP_ERROR_CHECK(nvs_flash_init()); // Retry nvs_flash_init
	}
	ESP_ERROR_CHECK(ret);
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

void init_wifi() {
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