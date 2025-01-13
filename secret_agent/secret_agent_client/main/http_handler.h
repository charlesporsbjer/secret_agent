#ifndef HTTP_HANDLER_H
#define HTTP_HANDLER_H

#include "shared_resources.h"
#include "printer_helper.h"
#include "mqtt_handler.h"
#include "esp_log.h"
#include "esp_http_client.h"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"

esp_err_t http_event_handler(esp_http_client_event_t *evt);





#endif