#include "client.h"
#include "printer_helper.h"
#include "esp_http_client.h"
<<<<<<< HEAD
#include "certs.h"
#include "esp_http_client.h" // Add this line to include the HTTP event enumerations
#include "mqtt_handler.h"
#include "serial.h"
#include "chat.h"
#include "freertos/queue.h"
#include "shared_resources.h"
#include "generate_csr.h"
#include "esp_event.h"
#include "esp_tls.h"

// Zainab 172.16.217.104
// Me 172.16.217.226
#define SERVER_IP "172.16.217.226" // Change this to the server IP address

#define SERVER_URL          "https://" SERVER_IP ":9191"
#define SERVER_REGISTER_URL "https://" SERVER_IP ":9191/spelare"
#define SERVER_START_URL    "https://" SERVER_IP ":9191/start"
#define CSR_ENDPOINT        "https://" SERVER_IP ":9191/spelare/csr"

#define MAX_HTTP_RECV_BUFFER 2048
#define MAX_HTTP_OUTPUT_BUFFER 2048

esp_mqtt_client_handle_t mqtt_client;

esp_err_t _http_event_handler(esp_http_client_event_t *evt) {
    static char *output_buffer;  // Buffer to store response of http request from event handler
    static int output_len;      // Stores number of bytes read

    switch (evt->event_id) {
        case HTTP_EVENT_ERROR:
            PRINTFC_CLIENT("HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            PRINTFC_CLIENT("HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            PRINTFC_CLIENT("HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            PRINTFC_CLIENT("HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            PRINTFC_CLIENT("HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            // Clean the buffer in case of a new request
            if (output_len == 0 && evt->user_data) {
                // we are just starting to copy the output data into the use
                memset(evt->user_data, 0, MAX_HTTP_OUTPUT_BUFFER);
            }
            /*
             *  Check for chunked encoding is added as the URL for chunked encoding used in this example returns binary data.
             *  However, event handler can also be used in case chunked encoding is used.
             */
            if (!esp_http_client_is_chunked_response(evt->client)) {
                // If user_data buffer is configured, copy the response into the buffer
                int copy_len = 0;
                if (evt->user_data) {
                    // The last byte in evt->user_data is kept for the NULL character in case of out-of-bound access.
                    copy_len = MIN(evt->data_len, (MAX_HTTP_OUTPUT_BUFFER - output_len));
                    if (copy_len) {
                        memcpy(evt->user_data + output_len, evt->data, copy_len);
                    }
                } else {
                    int content_len = esp_http_client_get_content_length(evt->client);
                    if (output_buffer == NULL) {
                        // We initialize output_buffer with 0 because it is used by strlen() and similar functions therefore should be null terminated.
                        output_buffer = (char *) calloc(content_len + 1, sizeof(char));
                        output_len = 0;
                        if (output_buffer == NULL) {
                            PRINTFC_CLIENT("Failed to allocate memory for output buffer");
                            return ESP_FAIL;
                        }
                    }
                    copy_len = MIN(evt->data_len, (content_len - output_len));
                    if (copy_len) {
                        memcpy(output_buffer + output_len, evt->data, copy_len);
                    }
                }
                output_len += copy_len;
            }
            break;
        case HTTP_EVENT_ON_FINISH:
            PRINTFC_CLIENT("HTTP_EVENT_ON_FINISH");
            char *cert_start = strstr((char*)output_buffer, "-----BEGIN CERTIFICATE-----");
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
                    // {"id": "%s"}
                    char *player_id_start = strstr((char*)output_buffer, "{\"id");
                    if (player_id_start) {
                        player_id_start += strlen("{\"id\": \"");
                        char *player_id_end = strstr(player_id_start, "\"}");
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
                        PRINTFC_CLIENT("Not a player ID response");
                    }
                }
            if (output_buffer != NULL) {
                free(output_buffer);
                output_buffer = NULL;
            }
            output_len = 0;
            break;
        case HTTP_EVENT_DISCONNECTED:
            PRINTFC_CLIENT("HTTP_EVENT_DISCONNECTED");
            int mbedtls_err = 0;
            esp_err_t err = esp_tls_get_and_clear_last_error((esp_tls_error_handle_t)evt->data, &mbedtls_err, NULL);
            if (err != 0) {
                PRINTFC_CLIENT("Last esp error code: 0x%x", err);
                PRINTFC_CLIENT("Last mbedtls failure: 0x%x", mbedtls_err);
            }
            if (output_buffer != NULL) {
                free(output_buffer);
                output_buffer = NULL;
            }
            output_len = 0;
            break;
        case HTTP_EVENT_REDIRECT:
            PRINTFC_CLIENT("HTTP_EVENT_REDIRECT");
            break;
        default:
            PRINTFC_CLIENT("Other event id: %d", evt->event_id);
            break;
    }
    return ESP_OK;
}
=======
#include "generate_csr.h"
#include "esp_err.h"


#define TAG "client"

esp_mqtt_client_handle_t mqtt_client;
>>>>>>> patriks

void client_task(void *p)
{
    client_init_param_t *param = (client_init_param_t *)p;
    PRINTFC_CLIENT("Client started and waiting for Wi-Fi to connect");
    // Wait for Wi-Fi to connect
<<<<<<< HEAD
    xEventGroupWaitBits(wifi_event_group, BIT0 | BIT1, pdFALSE, pdTRUE, portMAX_DELAY); // Wait for the Wi-Fi connected bit
=======
    xEventGroupWaitBits(wifi_event_group, BIT1, pdFALSE, pdTRUE, portMAX_DELAY); // Wait for the Wi-Fi connected bit
>>>>>>> patriks

    // Start serial task
    PRINTFC_CLIENT("Starting serial task");
    xTaskCreate(serial_task, "serial task", 16384, NULL, 5, NULL);

<<<<<<< HEAD
    // Register as a player
    PRINTFC_CLIENT("Sending player registration request");
    register_player();
=======
    xTaskCreate(serial_task, "serial task", 8192, NULL, 5, NULL);
>>>>>>> patriks

     register_player();
    // Generate and send CSR
<<<<<<< HEAD
    char csr[2048];
    PRINTFC_CLIENT("player_id before generating CSR: %s", player_id);
    generate_csr(csr, sizeof(csr), player_id); // Use the actual player ID
    PRINTFC_CLIENT("CSR: %s", csr);
    PRINTFC_CLIENT("Sending CSR");
    send_csr(csr);

    // Start the game when ready
    xEventGroupWaitBits(wifi_event_group, BIT0 | BIT1 | BIT2, pdFALSE, pdTRUE, portMAX_DELAY); // Wait for the signed certificate
    // PRINTFC_CLIENT("Sending game start request");
    // start_game();

    // Example MQTT initialization
    // Initialize the MQTT client and return the handle
    mqtt_client = mqtt_app_start();
    if (mqtt_client) {
        if (xSemaphoreTake(xSemaphore_mqtt_client, portMAX_DELAY) == pdTRUE) {
            PRINTFC_CLIENT("MQTT client initialized successfully.");
            mqtt_subscribe(mqtt_client);
            xSemaphoreGive(xSemaphore_mqtt_client);
        } else {
            PRINTFC_CLIENT("Failed to take MQTT semaphore.");
        }
    }
=======

     send_csr();

    mqtt_client = mqtt_app_start();
 
>>>>>>> patriks

    // Start chat task
  //  xTaskCreate(chat_task, "chat task", 8192, mqtt_client, 4, NULL);


    while(1){
        vTaskDelay(100/ portTICK_PERIOD_MS);
    }
    
    vTaskDelete(NULL);
}
void client_start(client_init_param_t *param)
{
    PRINTFC_CLIENT("Client starting");

    void *p = (void *)param;
    if (xTaskCreate(client_task, "client task", 16384, p, 5, NULL) != pdPASS) {
        PRINTFC_CLIENT("Failed to create client task");
    }
}

// Register ESP32 as a player
void register_player()
{
<<<<<<< HEAD
    //PRINTFC_CLIENT("ca_server_copy: %s", (const char*)ca_server_copy);
    PRINTFC_CLIENT("Register URL: %s", SERVER_REGISTER_URL);
    esp_http_client_config_t config = {
        .url = SERVER_REGISTER_URL,
        .cert_pem = (const char*)ca_server_copy, // Server's certificate for verification
        .skip_cert_common_name_check = true,
        .event_handler = _http_event_handler,
        .timeout_ms = 10000, // Increase timeout to 10 seconds
        .method = HTTP_METHOD_POST,
    };
=======
   esp_http_client_config_t config = {
            .url = SERVER_REGISTER,
            .cert_pem = (const char*)ca_cert_pem_start,
            .timeout_ms = 10000,
            .method = HTTP_METHOD_POST,
            .event_handler = http_event_handler,
        // .skip_cert_common_name_check = false,
                };
>>>>>>> patriks

    esp_http_client_handle_t client = esp_http_client_init(&config);

    if (client == NULL) {
        PRINTFC_CLIENT("Failed to initialize HTTP client");
        return;
    }
    char* json = "{}";
    size_t json_len =sizeof(json);

    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, json, json_len); 

    esp_err_t err = esp_http_client_perform(client);
<<<<<<< HEAD

    PRINTFC_CLIENT("Performing HTTP POST errno: %s", esp_err_to_name(err));

    esp_http_client_cleanup(client);
}


void send_csr(const char *csr)
{
    PRINTFC_CLIENT("CSR URL: %s", CSR_ENDPOINT);
    esp_http_client_config_t config = {
        .url = CSR_ENDPOINT,
        .cert_pem = (const char *)ca_server_copy,
        .skip_cert_common_name_check = true, // For testing only; remove in production
        .common_name = player_id,
        .event_handler = _http_event_handler,
        .timeout_ms = 10000, // Increase timeout to 10 seconds
=======
    esp_http_client_cleanup(client);
}

void send_csr()
{
    xEventGroupWaitBits(wifi_event_group, BIT10, pdFALSE, pdTRUE, portMAX_DELAY); // Wait for the Wi-Fi connected bit
    char csr[2048];
    generate_csr(csr, sizeof(csr), playerID); // Use the actual player ID

    esp_http_client_config_t config = {
        .url = CSR_ENDPOINT,
        .cert_pem = (const char*)ca_cert_pem_start     
>>>>>>> patriks
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
    } else {
    PRINTFC_CLIENT("Error sending CSR: %s", esp_err_to_name(err));
    }
<<<<<<< HEAD

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
        .event_handler = _http_event_handler,
        .common_name = player_id,
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

=======
>>>>>>> patriks
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



