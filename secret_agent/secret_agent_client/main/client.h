#ifndef CLIENT_H
#define CLIENT_H

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "mqtt_handler.h"
#include "freeRTOS/queue.h"
#include "shared_resources.h"
#include "certs.h"
#include "serial.h"
#include "chat.h"
#include "http_handler.h"

#define CSR_ENDPOINT "https://" SERVER_IP ":9191/spelare/csr"

#define SERVER_IP "192.168.0.127" //HEMMA
#define SERVER_REGISTER "https://" SERVER_IP ":9191/spelare/register"
#define SERVER_START "https://" SERVER_IP ":9191/start"
#define SERVER_TEST "https://" SERVER_IP ":9191/spelare/test"
#define SERVER_URL "https://" SERVER_IP ":9191/spelare/register"

typedef struct client_init_param_t {
    EventGroupHandle_t wifi_event_group;
} client_init_param_t;

void client_start(client_init_param_t *param);



void register_player();

void send_server_request();

void send_csr();



void mqtt_subscribe(esp_mqtt_client_handle_t client);

void mqtt_publish(esp_mqtt_client_handle_t client, const char *topic, const char *message);



#endif