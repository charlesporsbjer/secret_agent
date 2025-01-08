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

//#define SERVER_URL "https://localhost:9191/spelare/register"
//172.16.219.34
#define CSR_ENDPOINT "https://" SERVER_IP ":9191/spelare/csr"

//#define SERVER_IP "172.16.219.34"   //patrik
#define SERVER_IP "192.168.0.127" //HEMMA
#define SERVER_REGISTER "https://" SERVER_IP ":9191/spelare/register"
#define SERVER_START "https://" SERVER_IP ":9191/start"
#define SERVER_TEST "https://" SERVER_IP ":9191/spelare/test"
#define SERVER_URL "https://" SERVER_IP ":9191/spelare/register"

void client_task(void *p)
{
    client_init_param_t *param = (client_init_param_t *)p;
    PRINTFC_CLIENT("Client started and waiting for Wi-Fi to connect");
    // Wait for Wi-Fi to connect
    xEventGroupWaitBits(wifi_event_group, BIT0, pdFALSE, pdTRUE, portMAX_DELAY); // Wait for the Wi-Fi connected bit

    // Start serial task
    PRINTFC_CLIENT("Starting serial task");
    xTaskCreate(serial_task, "serial task", 16384, NULL, 5, NULL);
    PRINTFC_CLIENT("Returned from serial task");

  //  xTaskCreate(serial_task, "serial task", 8192, NULL, 5, NULL);

    // Register as a player
   // register_player();
    // Generate and send CSR
 
    //generate_csr(csr, sizeof(csr), "p1"); // Use the actual player ID
   // send_csr();

    // Start the game when ready
    //start_game();



    // Start chat task
   // xTaskCreate(chat_task, "chat task", 8192, mqtt_client, 4, NULL);

    while (1)
    {
        send_server_request();
    
        // Periodic task or logic (e.g., game state updates, handling MQTT messages)
        vTaskDelay(pdMS_TO_TICKS(5000)); // Adjust the delay as needed
    }

    vTaskDelete(NULL);
}

void send_server_request(){

        esp_http_client_config_t config = {
            .url = SERVER_TEST,
            .cert_pem = (const char*)ca_cert_pem_start,
            .timeout_ms = 10000,
        // .skip_cert_common_name_check = false,
                };
        

        esp_http_client_handle_t client = esp_http_client_init(&config);

        esp_http_client_set_method(client, HTTP_METHOD_POST);
        esp_http_client_init

        // Perform the HTTP request
        esp_err_t err = esp_http_client_perform(client);


    
        int content_length = esp_http_client_get_content_length(client);


        
        
        if (err == ESP_OK) {
            PRINTFC_CLIENT("HTTP POST Status = %d, content_length = %lld\n",
                            esp_http_client_get_status_code(client),
                            esp_http_client_get_content_length(client));
                
                
                    char buffer[512]; // Adjust the size based on expected response size
                    int read_len = esp_http_client_read(client, buffer, content_length);
        
            if (content_length > 0) {
                buffer[read_len] = '\0';
                printf("Response: %s\n", buffer);

                printf("Raw response (hex): ");
                    for (int i = 0; i < read_len; i++) {
                        printf("%02X ", buffer[i]);
                    }
                    printf("\n");                
            
            } else {
            PRINTFC_CLIENT("Error performing HTTP POST: %s\n", esp_err_to_name(err));
            }


        esp_http_client_cleanup(client);

    }
}

/*
static void http_native_request(void)
{
    // Declare local_response_buffer with size (MAX_HTTP_OUTPUT_BUFFER + 1) to prevent out of bound access when
    // it is used by functions like strlen(). The buffer should only be used upto size MAX_HTTP_OUTPUT_BUFFER
    char output_buffer[MAX_HTTP_OUTPUT_BUFFER + 1] = {0};   // Buffer to store response of http request
    int content_length = 0;
    esp_http_client_config_t config = {
        .url = "http://"CONFIG_EXAMPLE_HTTP_ENDPOINT"/get",
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    // GET Request
    esp_http_client_set_method(client, HTTP_METHOD_GET);
    esp_err_t err = esp_http_client_open(client, 0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
    } else {
        content_length = esp_http_client_fetch_headers(client);
        if (content_length < 0) {
            ESP_LOGE(TAG, "HTTP client fetch headers failed");
        } else {
            int data_read = esp_http_client_read_response(client, output_buffer, MAX_HTTP_OUTPUT_BUFFER);
            if (data_read >= 0) {
                ESP_LOGI(TAG, "HTTP GET Status = %d, content_length = %"PRId64,
                esp_http_client_get_status_code(client),
                esp_http_client_get_content_length(client));
                ESP_LOG_BUFFER_HEX(TAG, output_buffer, data_read);
            } else {
                ESP_LOGE(TAG, "Failed to read response");
            }
        }
    }
    esp_http_client_close(client);

    // POST Request
    const char *post_data = "{\"field1\":\"value1\"}";
    esp_http_client_set_url(client, "http://"CONFIG_EXAMPLE_HTTP_ENDPOINT"/post");
    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    err = esp_http_client_open(client, strlen(post_data));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
    } else {
        int wlen = esp_http_client_write(client, post_data, strlen(post_data));
        if (wlen < 0) {
            ESP_LOGE(TAG, "Write failed");
        }
        content_length = esp_http_client_fetch_headers(client);
        if (content_length < 0) {
            ESP_LOGE(TAG, "HTTP client fetch headers failed");
        } else {
            int data_read = esp_http_client_read_response(client, output_buffer, MAX_HTTP_OUTPUT_BUFFER);
            if (data_read >= 0) {
                ESP_LOGI(TAG, "HTTP POST Status = %d, content_length = %"PRId64,
                esp_http_client_get_status_code(client),
                esp_http_client_get_content_length(client));
                ESP_LOG_BUFFER_HEX(TAG, output_buffer, strlen(output_buffer));
            } else {
                ESP_LOGE(TAG, "Failed to read response");
            }
        }
    }
    esp_http_client_cleanup(client);
}




*/





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
        .url = SERVER_URL,
        //.cert_pem = (const char*)ca_cert_pem_start, // Server's certificate for verification
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);

    if (client == NULL) {
        PRINTFC_CLIENT("Failed to initialize HTTP client\n");
        return;
    }

    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK) {
        PRINTFC_CLIENT("HTTP POST Status = %d, content_length = %lld\n",
               esp_http_client_get_status_code(client),
               esp_http_client_get_content_length(client));

        // Handle response
        char response[128];
        int content_length = esp_http_client_read(client, response, sizeof(response) - 1);
        if (content_length > 0) {
            response[content_length] = '\0';
            PRINTFC_CLIENT("Response: %s\n", response);

            // Parse and store player ID if needed
        }
    } else {
        PRINTFC_CLIENT("Error performing HTTP POST: %s\n", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
}

void send_csr()
{
    char csr[2048];
    generate_csr(csr, sizeof(csr), "p1"); // Use the actual player ID

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

void start_game()
{
    const char *json_payload = "{\"val\": \"nu kör vi\"}";

    esp_http_client_config_t config = {
        .url = SERVER_START,
        .cert_pem = (const char*)ca_cert_pem_start,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);

    if (client == NULL) {
        PRINTFC_CLIENT("Failed to initialize HTTP client\n");
        return;
    }

    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_post_field(client, json_payload, strlen(json_payload));
    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK) {
        PRINTFC_CLIENT("Game start request sent successfully.\n");
    } else {
        PRINTFC_CLIENT("Error starting game: %s\n", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
}

void mqtt_subscribe(esp_mqtt_client_handle_t client)
{   
    if (xSemaphoreTake(xSemaphore_mqtt_client, portMAX_DELAY) == pdTRUE) {
        int msg_id = esp_mqtt_client_subscribe(client, "/myndigheten", 0);
        if (msg_id != -1) {
            PRINTFC_CLIENT("Subscribed to /myndigheten successfully.\n");
        } else {
            PRINTFC_CLIENT("Subscription failed.\n");
        }
        xSemaphoreGive(xSemaphore_mqtt_client);
    } else {
        PRINTFC_CLIENT("Failed to take MQTT semaphore.\n");
    }
}

void mqtt_publish(esp_mqtt_client_handle_t client, const char *topic, const char *message)
{   
    if (xSemaphoreTake(xSemaphore_mqtt_client, portMAX_DELAY) == pdTRUE) {
        int msg_id = esp_mqtt_client_publish(client, topic, message, 0, 1, 0);
        if (msg_id != -1) {
            PRINTFC_CLIENT("Message published to %s.\n", topic);
        } else {
            PRINTFC_CLIENT("Publish failed.\n");
        }
        xSemaphoreGive(xSemaphore_mqtt_client);
    } 
    else {
        PRINTFC_CLIENT("Failed to take MQTT semaphore.\n");
    }    
}

esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
    switch (event->event_id) {
        case MQTT_EVENT_DATA:
            PRINTFC_CLIENT("Received data: Topic=%.*s, Message=%.*s\n",
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


    // Example MQTT initialization
    // Initialize the MQTT client and return the handle
    // esp_mqtt_client_handle_t mqtt_client = mqtt_app_start();
    // if (mqtt_client) {
    //     if (xSemaphoreTake(xSemaphore_mqtt_client, portMAX_DELAY) == pdTRUE) {
    //         PRINTFC_CLIENT("MQTT client initialized successfully.");
    //         mqtt_subscribe(mqtt_client);
    //         xSemaphoreGive(xSemaphore_mqtt_client);
    //     } else {
    //         PRINTFC_CLIENT("Failed to take MQTT semaphore.");
    //     }
    // }
