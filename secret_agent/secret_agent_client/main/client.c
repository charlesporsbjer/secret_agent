#include "client.h"
#include "printer_helper.h"
#include "mbedtls/x509_csr.h"
#include "mbedtls/pem.h"
#include "esp_http_client.h"
#include "certs.h"
#include "mqtt_handler.h"
#include "serial.h"
#include "chat.h"
#include "freertos/queue.h"
#include "shared_resources.h"
#include "generate_csr.h"

#define SERVER_IP "192.168.2.206" // Change this to the server IP address

#define SERVER_URL          "https://" SERVER_IP ":9191"
#define SERVER_REGISTER_URL "https://" SERVER_IP ":9191/spelare"
#define SERVER_START_URL    "https://" SERVER_IP ":9191/start"
#define CSR_ENDPOINT        "https://" SERVER_IP ":9191/spelare/csr"

void client_task(void *p)
{
    client_init_param_t *param = (client_init_param_t *)p;
    PRINTFC_CLIENT("Client started and waiting for Wi-Fi to connect");
    // Wait for Wi-Fi to connect
    xEventGroupWaitBits(wifi_event_group, BIT0 | BIT1, pdFALSE, pdTRUE, portMAX_DELAY); // Wait for the Wi-Fi connected bit

    // Start serial task
    PRINTFC_CLIENT("Starting serial task");
    xTaskCreate(serial_task, "serial task", 16384, NULL, 5, NULL);

    // Register as a player
    register_player();
    PRINTFC_CLIENT("Player register request sent");

    // Generate and send CSR
    char csr[2048];
    generate_csr(csr, sizeof(csr), player_id); // Use the actual player ID
    send_csr(csr);
    PRINTFC_CLIENT("CSR sent");

    // Start the game when ready
    start_game();
    PRINTFC_CLIENT("Game start request sent");

    // Example MQTT initialization
    // Initialize the MQTT client and return the handle
    esp_mqtt_client_handle_t mqtt_client = mqtt_app_start();
    if (mqtt_client) {
        if (xSemaphoreTake(xSemaphore_mqtt_client, portMAX_DELAY) == pdTRUE) {
            PRINTFC_CLIENT("MQTT client initialized successfully.");
            mqtt_subscribe(mqtt_client);
            xSemaphoreGive(xSemaphore_mqtt_client);
        } else {
            PRINTFC_CLIENT("Failed to take MQTT semaphore.");
        }
    }

    // Start chat task
    if (xTaskCreate(chat_task, "chat task", 8192, (void*)mqtt_client, 4, NULL) != pdPASS) 
    {
        PRINTFC_CLIENT("Failed to create chat task");
        vTaskDelete(NULL);
        return;
    }

    while (1)
    {
        // Periodic task or logic (e.g., game state updates, handling MQTT messages)
        vTaskDelay(pdMS_TO_TICKS(1000)); // Adjust the delay as needed
    }

    vTaskDelete(NULL);
}

void client_start(client_init_param_t *param)
{
    PRINTFC_CLIENT("Client starting");
    
    // // Check if there is a connection to the server by sending a GET request to server URL
    // PRINTFC_CLIENT("Server URL: %s", SERVER_URL);
    // esp_http_client_config_t config = {
    //     .url = SERVER_URL,
    //     .cert_pem = (const char*)ca_server_copy, // Server's certificate for verification
    //     .skip_cert_common_name_check = true,
    // };
    // if (esp_http_client_perform(esp_http_client_init(&config)) != ESP_OK) {
    //     PRINTFC_CLIENT("Failed to connect to the server");
    // } else {
    //     PRINTFC_CLIENT("Connected to the server");
    // }

    void *p = (void *)param;
    if (xTaskCreate(client_task, "client task", 16384, p, 5, NULL) != pdPASS) {
        PRINTFC_CLIENT("Failed to create client task");
    }
}

// Register ESP32 as a player
void register_player()
{
    PRINTFC_CLIENT("ca_server_copy: %s", (const char*)ca_server_copy);
    PRINTFC_CLIENT("Register URL: %s", SERVER_REGISTER_URL);
    esp_http_client_config_t config = {
        .url = SERVER_REGISTER_URL,
        .cert_pem = (const char*)ca_server_copy, // Server's certificate for verification
        .skip_cert_common_name_check = true,
        //.common_name = "http.creekside.se",
        .auth_type = HTTP_AUTH_TYPE_NONE,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);

    if (client == NULL) {
        PRINTFC_CLIENT("Failed to initialize HTTP client");
        return;
    }

    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK) {
        PRINTFC_CLIENT("HTTP POST Status = %d, content_length = %lld",
                       esp_http_client_get_status_code(client),
                       esp_http_client_get_content_length(client));

        // Handle response
        char response[128];
        int content_length = esp_http_client_read(client, response, sizeof(response) - 1);
        if (content_length > 0) {
            response[content_length] = '\0';
            PRINTFC_CLIENT("Response: %s", response);

            // Parse and store player ID json string
            char *player_id_start = strstr(response, "\"id\": \"");
            if (player_id_start) {
                player_id_start += strlen("\"id\": \"");
                char *player_id_end = strstr(player_id_start, "\"");
                if (player_id_end) {
                    int player_id_len = player_id_end - player_id_start;
                    if (player_id_len < sizeof(player_id)) {
                        memset(player_id, 0, sizeof(player_id));
                        strncpy(player_id, player_id_start, player_id_len);
                        player_id[player_id_len] = '\0'; // Ensure null-termination
                        PRINTFC_CLIENT("Player ID: %s", player_id);
                    } else {
                        PRINTFC_CLIENT("Error: Player ID buffer too small");
                    }
                } else {
                    PRINTFC_CLIENT("Error: Missing end quote for player ID");
                }
            } else {
                PRINTFC_CLIENT("Error: No player ID found in response");
            }
        } else {
            PRINTFC_CLIENT("Error reading response or no content");
        }
    } else {
        PRINTFC_CLIENT("Error performing HTTP POST: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
}


void send_csr(const char *csr)
{
    PRINTFC_CLIENT("CSR URL: %s", CSR_ENDPOINT);
    esp_http_client_config_t config = {
        .url = CSR_ENDPOINT,
        .cert_pem = (const char *)ca_server_copy,
        //.skip_cert_common_name_check = true, // For testing only; remove in production
        .common_name = player_id,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (client == NULL) {
        PRINTFC_CLIENT("Failed to initialize HTTP client");
        return;
    }

    // Configure HTTP method and payload
    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_post_field(client, csr, strlen(csr));

    // Perform the HTTP request
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        PRINTFC_CLIENT("CSR submitted successfully.");
        PRINTFC_CLIENT("HTTP Status = %d, Content-Length = %" PRId64,
                       esp_http_client_get_status_code(client),
                       esp_http_client_get_content_length(client));

        // Handle the server response
        char response[2048];
        int content_length = esp_http_client_read(client, response, sizeof(response) - 1);
        if (content_length > 0) {
            response[content_length] = '\0'; // Null-terminate the response
            PRINTFC_CLIENT("Response: %s", response);

            // Parse the signed certificate from the response
            char *cert_start = strstr(response, "-----BEGIN CERTIFICATE-----");
            if (cert_start) {
                char *cert_end = strstr(cert_start, "-----END CERTIFICATE-----");
                if (cert_end) {
                    cert_end += strlen("-----END CERTIFICATE-----"); // Move to the end of the certificate

                    int cert_len = cert_end - cert_start;
                    if (cert_len < sizeof(signed_certificate)) {
                        memset(signed_certificate, 0, sizeof(signed_certificate));
                        strncpy(signed_certificate, cert_start, cert_len);
                        signed_certificate[cert_len] = '\0'; // Ensure null-termination

                        PRINTFC_CLIENT("Signed certificate: %s", signed_certificate);

                        // Signal that the signed certificate has been received
                        xEventGroupSetBits(wifi_event_group, BIT2);
                    } else {
                        PRINTFC_CLIENT("Error: Signed certificate buffer too small");
                    }
                } else {
                    PRINTFC_CLIENT("Error: Missing 'END CERTIFICATE' marker");
                }
            } else {
                PRINTFC_CLIENT("Error: No certificate found in the response");
            }
        } else {
            PRINTFC_CLIENT("Error: Failed to read server response");
        }
    } else {
        PRINTFC_CLIENT("Error sending CSR: %s", esp_err_to_name(err));
    }

    // Clean up the HTTP client
    esp_http_client_cleanup(client);
}


void start_game()
{
    PRINTFC_CLIENT("Game start URL: %s", SERVER_START_URL);
    const char *json_payload = "{\"val\": \"nu kör vi\"}";

    esp_http_client_config_t config = {
        .url = SERVER_START_URL,
        .cert_pem = (const char*)signed_certificate,
        .skip_cert_common_name_check = true,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);

    if (client == NULL) {
        PRINTFC_CLIENT("Failed to initialize HTTP client");
        return;
    }

    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_post_field(client, json_payload, strlen(json_payload));
    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK) {
        PRINTFC_CLIENT("Game start request sent successfully.");
    } else {
        PRINTFC_CLIENT("Error starting game: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
}

void mqtt_subscribe(esp_mqtt_client_handle_t client)
{   
    if (xSemaphoreTake(xSemaphore_mqtt_client, portMAX_DELAY) == pdTRUE) {
        int msg_id = esp_mqtt_client_subscribe(client, "/myndigheten", 0);
        if (msg_id != -1) {
            PRINTFC_CLIENT("Subscribed to /myndigheten successfully.");
        } else {
            PRINTFC_CLIENT("Subscription failed.");
        }
        xSemaphoreGive(xSemaphore_mqtt_client);
    } else {
        PRINTFC_CLIENT("Failed to take MQTT semaphore.");
    }
}

void mqtt_publish(esp_mqtt_client_handle_t client, const char *topic, const char *message)
{   
    if (xSemaphoreTake(xSemaphore_mqtt_client, portMAX_DELAY) == pdTRUE) {
        int msg_id = esp_mqtt_client_publish(client, topic, message, 0, 1, 0);
        if (msg_id != -1) {
            PRINTFC_CLIENT("Message published to %s.", topic);
        } else {
            PRINTFC_CLIENT("Publish failed.");
        }
        xSemaphoreGive(xSemaphore_mqtt_client);
    } 
    else {
        PRINTFC_CLIENT("Failed to take MQTT semaphore.");
    }    
}

esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
    switch (event->event_id) {
        case MQTT_EVENT_DATA:
            PRINTFC_CLIENT("Received data: Topic=%.*s, Message=%.*s",
                   event->topic_len, event->topic,
                   event->data_len, event->data);
            // Process the message (e.g., update game state)
            if (xSemaphoreTake(xSemaphore_mqtt_evt, portMAX_DELAY) == pdTRUE) 
            {   
                xQueueSend(mqtt_event_queue, event, portMAX_DELAY); // Send the event to the queue
                xSemaphoreGive(xSemaphore_mqtt_evt); // Give back the semaphore
            }
            break;
        default:
            break;
    }
    return ESP_OK;
}
