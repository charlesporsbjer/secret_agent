#include <stdio.h>
#include <string.h>
#include "mqtt_client.h"

static esp_mqtt_client_handle_t client;

void mqtt_event_handler(esp_mqtt_event_handle_t event) {
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            printf("MQTT Connected\n");
            break;
        case MQTT_EVENT_DISCONNECTED:
            printf("MQTT Disconnected\n");
            break;
        case MQTT_EVENT_SUBSCRIBED:
            printf("Subscribed to topic\n");
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            printf("Unsubscribed from topic\n");
            break;
        case MQTT_EVENT_DATA:
            printf("Received data: %.*s\n", event->data_len, event->data);
            break;
        case MQTT_EVENT_ERROR:
            printf("MQTT Error\n");
            break;
        default:
            break;
    }
}

void mqtt_client_init(const char *uri) {
    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = uri,
        .event_handle = mqtt_event_handler,
    };

    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_start(client);
}

void mqtt_client_publish(const char *topic, const char *data) {
    esp_mqtt_client_publish(client, topic, data, 0, 1, 0);
}

void mqtt_client_subscribe(const char *topic) {
    esp_mqtt_client_subscribe(client, topic, 0);
}

void mqtt_client_disconnect() {
    esp_mqtt_client_stop(client);
}