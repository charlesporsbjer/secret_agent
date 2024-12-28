#include "server.h"
#include "client.h"
#include "wifi_handler.h"
#include "printer_helper.h"
#include "mqtt_handler.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#define SERIAL_MSG_BUF_SIZE 1024

SemaphoreHandle_t xSemaphore_wifi_event;
SemaphoreHandle_t xSemaphore_serial;
SemaphoreHandle_t xSemaphore_mqtt_evt;
SemaphoreHandle_t xSemaphore_mqtt_client;

QueueHandle_t mqtt_event_queue;
QueueHandle_t serial_msg_queue;

EventGroupHandle_t wifi_event_group;
client_init_param_t c_param;
server_init_param_t s_param;

wifi_init_param_t w_param = {
    .ssid = CONFIG_WIFI_SSID,
    .password = CONFIG_WIFI_PASSWORD,
    
};

void app_main(void)
{
    PRINTFC_MAIN("Main is starting");

    PRINTFC_MAIN("NVS Initialize");
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        nvs_flash_erase();
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    err = nvs_flash_init_partition("eol");
    ESP_ERROR_CHECK(err);

    ESP_ERROR_CHECK(esp_event_loop_create_default());

    PRINTFC_MAIN("Creating event group");
    wifi_event_group = xEventGroupCreate();
    xSemaphore_wifi_event = xSemaphoreCreateMutex();
    s_param.wifi_event_group = wifi_event_group;
    w_param.wifi_event_group = wifi_event_group;
    c_param.wifi_event_group = wifi_event_group;

    // Create the queues
    serial_msg_queue = xQueueCreate(10, sizeof(char) * SERIAL_MSG_BUF_SIZE);
    xSemaphore_serial = xSemaphoreCreateMutex();
    
    mqtt_event_queue = xQueueCreate(10, sizeof(esp_mqtt_event_handle_t));
    xSemaphore_mqtt_evt = xSemaphoreCreateMutex();
    xSemaphore_mqtt_client = xSemaphoreCreateMutex();

    PRINTFC_MAIN("Starting all tasks");
    wifi_handler_start(&w_param);
    server_start(&s_param);
    client_start(&c_param);
    PRINTFC_MAIN("Main is done");
}