#ifndef MQTT_HANDLER_H
#define MQTT_HANDLER_H

#include "mqtt_client.h"

esp_mqtt_client_handle_t mqtt_app_start();

void mqtt_message_handler(void *event_data);

#endif