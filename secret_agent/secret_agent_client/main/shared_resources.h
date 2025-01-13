#ifndef SHARED_RESOURCES_H
#define SHARED_RESOURCES_H

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#define GOT_PLAYER_ID_BIT BIT10

extern SemaphoreHandle_t xSemaphore_serial;
extern SemaphoreHandle_t xSemaphore_mqtt_evt;
extern SemaphoreHandle_t xSemaphore_mqtt_client;
extern SemaphoreHandle_t xSemaphore_wifi_event;

extern QueueHandle_t mqtt_event_queue;
extern QueueHandle_t serial_msg_queue;
extern EventGroupHandle_t wifi_event_group;

#endif // SHARED_RESOURCES_H