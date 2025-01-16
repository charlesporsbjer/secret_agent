#ifndef CLIENT_H
#define CLIENT_H

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "mqtt_client.h"
#include "shared_resources.h"


void client_start();

void register_player();

void send_csr(const char *csr);

void start_game();


#endif