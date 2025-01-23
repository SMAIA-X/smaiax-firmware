/*
 * data_sender.c
 *
 *  Created on: 21.01.2025
 *      Author: michael
 */
 
#include "data_sender.h"
#include "mqtt_client.h"
#include "esp_log.h"

#define SENDER_TAG "SENDER"

esp_mqtt_client_handle_t client;

static void log_error_if_nonzero(const char *message, int error_code) {
    if (error_code != 0) {
        ESP_LOGE(SENDER_TAG, "Last error %s: 0x%x", message, error_code);
    }
}

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
			ESP_LOGE(SENDER_TAG, "MQTT_ERROR_TYPE_TCP_TRANSPORT");
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

void sendData(measurement_t* measurement) {
	char json[512];
 	char timestamp[30];
    snprintf(timestamp, sizeof(timestamp), "%04d-%02d-%02dT%02d:%02d:%02d",
             measurement->year, measurement->month, measurement->day,
             measurement->hour, measurement->minute, measurement->second);

	int len = snprintf(json, sizeof(json),
        "{"
        "\"timestamp\":\"%s\","
        "\"voltage_l1\":%.2f,"
        "\"voltage_l2\":%.2f,"
        "\"voltage_l3\":%.2f,"
        "\"current_l1\":%.2f,"
        "\"current_l2\":%.2f,"
        "\"current_l3\":%.2f,"
        "\"active_power_plus\":%.2f,"
        "\"active_power_minus\":%.2f,"
        "\"reactive_power_plus\":%.2f,"
        "\"reactive_power_minus\":%.2f,"
        "\"active_energy_plus\":%.2f,"
        "\"active_energy_minus\":%.2f"
        "}",
        timestamp,
        measurement->voltage_phase_1,
        measurement->voltage_phase_2,
        measurement->voltage_phase_3,
        measurement->current_phase_1,
        measurement->current_phase_2,
        measurement->current_phase_3,
        measurement->positive_active_power,
        measurement->negative_active_power,
        measurement->reactive_power_plus,
        measurement->reactive_power_minus,
        measurement->positive_active_energy_total,
        measurement->negative_active_energy_total);
        
 	if (len < 0 || len >= sizeof(json)) {
        ESP_LOGE(SENDER_TAG, "JSON string is too large for buffer");
        return;
    }
	
	int msg_id = esp_mqtt_client_publish(client, "/topic/qos0", json, 0, 0, 0);
			
	if (msg_id != 0) {
		ESP_LOGE(SENDER_TAG, "Publish failed, msg_id=%d", msg_id);
	} else {
		ESP_LOGI(SENDER_TAG, "Publish successful, msg_id=%d", msg_id);	
	}
}