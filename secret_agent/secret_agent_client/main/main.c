#include "client.h"
#include "serial.h"
#include "wifi_handler.h"
#include "printer_helper.h"
#include "mqtt_handler.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "certs.h"

#define SERIAL_MSG_BUF_SIZE 1024

char* signed_certificate[2048];
char* shorter_id[32] = {0};
char* playerID[256] = {0};

//#define SSID "STI Student"
//#define PASS "STI1924stu"

QueueHandle_t mqtt_event_queue;
QueueHandle_t serial_msg_queue;

EventGroupHandle_t wifi_event_group;
client_init_param_t c_param;

wifi_init_param_t w_param = {
    .ssid = CONFIG_WIFI_SSID,
    .password = CONFIG_WIFI_PASSWORD,
    
};
void print_embedded_certificate(void)
{
    PRINTFC_MAIN("Embedded certificate: %s", (const char*)ca_cert_pem_start);

}

void app_main(void)
{
    esp_log_level_set("wifi", ESP_LOG_WARN); 
    PRINTFC_MAIN("Main is starting");

    esp_log_level_set("wifi", ESP_LOG_ERROR);

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
 
    //w_param.wifi_event_group = wifi_event_group;
    //c_param.wifi_event_group = wifi_event_group;

    // Create the queues
    serial_msg_queue = xQueueCreate(10, sizeof(char) * SERIAL_MSG_BUF_SIZE); 
    mqtt_event_queue = xQueueCreate(10, sizeof(esp_mqtt_event_handle_t));
  
    
    print_embedded_certificate();

    PRINTFC_MAIN("Starting all tasks");
    wifi_init_start(&w_param);

    client_start();
    vTaskDelay(pdMS_TO_TICKS(1000));
    PRINTFC_MAIN("Main is done");
}