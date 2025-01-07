#ifndef CLIENT_H
#define CLIENT_H

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "mqtt_client.h"
#include "freeRTOS/queue.h"
#include "shared_resources.h"

typedef struct client_init_param_t {
    EventGroupHandle_t wifi_event_group;
} client_init_param_t;

void client_start(client_init_param_t *param);



void register_player();

void send_server_request();

void send_csr(const char *csr);

void start_game();

void mqtt_subscribe(esp_mqtt_client_handle_t client);

void mqtt_publish(esp_mqtt_client_handle_t client, const char *topic, const char *message);



#endif