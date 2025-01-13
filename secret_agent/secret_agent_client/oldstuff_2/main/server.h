#ifndef SERVER_H
#define SERVER_H

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#define END_OF_LINE_PARTITION "eol"
#define CERTIFICATE_NAMESPACE "certs"
#define CERTIFICATE_NAME "cert"
#define KEY_NAME "key"
#define PORT "9231"

typedef struct server_init_param_t
{
    EventGroupHandle_t wifi_event_group;
    // int wifi_event_group,
} server_init_param_t;

void server_start(server_init_param_t *param);

#endif