#ifndef CHAT_H
#define CHAT_H

#include "stdbool.h"
#include "mqtt_client.h"
#include "freertos/FreeRTOS.h"
#include "shared_resources.h"

void chat_task(void* mqtt_client);

#endif