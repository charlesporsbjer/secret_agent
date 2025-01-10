#ifndef SHARED_RESOURCES_H
#define SHARED_RESOURCES_H

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "mbedtls/pk.h"
#include "mqtt_client.h"

#define TOPIC_TORGET "/torget"
#define TOPIC_PLAYER_Uplink "/spelare/%d/uplink"
#define TOPIC_PLAYER_Downlink "/spelare/%d/downlink"
#define TOPIC_MYNDIGHETEN "/myndigheten"

extern SemaphoreHandle_t xSemaphore_serial;
extern SemaphoreHandle_t xSemaphore_mqtt_evt;
extern SemaphoreHandle_t xSemaphore_mqtt_client;
extern SemaphoreHandle_t xSemaphore_wifi_event;

extern QueueHandle_t mqtt_event_queue;
extern QueueHandle_t serial_msg_queue;
extern EventGroupHandle_t wifi_event_group;

extern char player_id[256];
extern char shorter_id[32];

extern char signed_certificate[2048]; 
extern uint8_t key_pem[2048];

extern esp_mqtt_client_handle_t mqtt_client;


#endif // SHARED_RESOURCES_H