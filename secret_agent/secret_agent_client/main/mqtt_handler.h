#ifndef MQTT_HANDLER_H
#define MQTT_HANDLER_H

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "printer_helper.h"
#include "certs.h"
#include "client.h"
#include "esp_event.h"
#include "shared_resources.h"


void mqtt_message_handler(void *pvParameters);

esp_mqtt_client_handle_t mqtt_app_start();  


#define MQTT_BROKER_URI "mqtts://"SERVER_IP":8885"


#endif