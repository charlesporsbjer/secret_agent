#ifndef SHARED_RESOURCES_H
#define SHARED_RESOURCES_H

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "mqtt_client.h"

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_HAS_IP_BIT BIT1
#define GOT_PLAYER_ID_BIT BIT2
#define GOT_CERTIFICATE_BIT BIT3

extern SemaphoreHandle_t xSemaphore_serial;
extern SemaphoreHandle_t xSemaphore_mqtt_evt;
extern SemaphoreHandle_t xSemaphore_mqtt_client;
extern SemaphoreHandle_t xSemaphore_wifi_event;

extern char* shorter_id[32];
extern char* playerID[256];
extern char* signed_certificate[2048];
extern uint8_t key_pem[2048];

extern QueueHandle_t mqtt_event_queue;
extern QueueHandle_t serial_msg_queue;
extern EventGroupHandle_t wifi_event_group;




#endif // SHARED_RESOURCES_H