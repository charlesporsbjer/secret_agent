#include "client.h"
#include "printer_helper.h"
#include "mbedtls/x509_csr.h"
#include "mbedtls/pem.h"
#include "esp_http_client.h"
#include "generate_csr.h"
#include "esp_err.h"


#define TAG "client"

esp_mqtt_client_handle_t mqtt_client;

void client_task(void *p)
{
    client_init_param_t *param = (client_init_param_t *)p;
    PRINTFC_CLIENT("Client started and waiting for Wi-Fi to connect");
    // Wait for Wi-Fi to connect
    xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT | WIFI_HAS_IP_BIT, pdFALSE, pdTRUE, portMAX_DELAY); 

    // Start serial task
  //  PRINTFC_CLIENT("Starting serial task");
   // xTaskCreate(serial_task, "serial task", 16384, NULL, 5, NULL);
  //  PRINTFC_CLIENT("Returned from serial task");

  //  xTaskCreate(serial_task, "serial task", 8192, NULL, 5, NULL);

    // Generate and send CSR

   //  send_csr();

  //  mqtt_client = mqtt_app_start();
 

    // Start chat task
  //  xTaskCreate(chat_task, "chat task", 8192, mqtt_client, 4, NULL);

    register_player();

    while(1){
    
        vTaskDelay(50/ portTICK_PERIOD_MS);
    }
    
    vTaskDelete(NULL);
}
void client_start(client_init_param_t *param)
{
    void *p = (void *)param;
    if (xTaskCreate(client_task, "client task", 16384, p, 5, NULL) != pdPASS) {
        PRINTFC_CLIENT("Failed to create client task");
    }
}

// Register ESP32 as a player
void register_player()
{
   esp_http_client_config_t config = {
            .url = SERVER_REGISTER,
            .cert_pem = (const char*)ca_cert_pem_start,
            .timeout_ms = 10000,
            .method = HTTP_METHOD_POST,
            .event_handler = http_event_handler,
            .transport_type = HTTP_TRANSPORT_OVER_SSL,
            .skip_cert_common_name_check = true,
                };

    esp_http_client_handle_t client = esp_http_client_init(&config);

    if (client == NULL) {
        PRINTFC_CLIENT("Failed to initialize HTTP client\n");
        return;
    }

    char* json = "{}";
    size_t json_len =sizeof(json);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, json, json_len);

    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        PRINTFC_CLIENT("Player registered successfully.\n");
    } else {
        PRINTFC_CLIENT("Error registering player: %s\n", esp_err_to_name(err));
    }
    esp_http_client_cleanup(client);
}

void send_csr()
{
    xEventGroupWaitBits(wifi_event_group, GOT_PLAYER_ID_BIT, pdFALSE, pdTRUE, portMAX_DELAY); // Wait for the Wi-Fi connected bit
    char csr[2048];
    generate_csr(csr, sizeof(csr), playerID); // Use the actual player ID

    esp_http_client_config_t config = {
        .url = CSR_ENDPOINT,
        .cert_pem = (const char*)ca_cert_pem_start     
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);

    if (client == NULL) {
        PRINTFC_CLIENT("Failed to initialize HTTP client\n");
        return;
    }

    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_post_field(client, csr, strlen(csr));
    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK) {
        PRINTFC_CLIENT("CSR submitted successfully.\n");
        // Handle server response if needed (signed certificate)
    } else {
        PRINTFC_CLIENT("Error sending CSR: %s\n", esp_err_to_name(err));
    }
    esp_http_client_cleanup(client);
}

void http_test(){
    
    esp_http_client_config_t config = {
            .url = SERVER_REGISTER,
            .cert_pem = (const char*)ca_cert_pem_start,
            .timeout_ms = 10000,
            .method = HTTP_METHOD_POST,
            .transport_type = HTTP_TRANSPORT_OVER_SSL,
            .skip_cert_common_name_check = true,
                };

    esp_http_client_handle_t client = esp_http_client_init(&config);

    if (client == NULL) {
        PRINTFC_CLIENT("Failed to initialize HTTP client\n");
        return;
    }

    char* json = "{}";
    size_t json_len =sizeof(json);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, json, json_len);

    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        PRINTFC_CLIENT("Player registered successfully.\n");
    } else {
        PRINTFC_CLIENT("Error registering player: %s\n", esp_err_to_name(err));
    }
    esp_http_client_cleanup(client);
}



