#include "http_handler.h"
#include "cJson.h"

static const char *TAG = "HTTP_HANDLER";


esp_err_t http_event_handler(esp_http_client_event_t *evt){

    static char *output_buffer;  // Buffer to store response of http request from event handler
    static int output_len = 0;   // Stores number of bytes read
    switch(evt->event_id){

        case HTTP_EVENT_ERROR:
            ESP_LOGI(TAG, "HTTP_EVENT_ERROR");
            PRINTFC_CLIENT("HTTP_EVENT_ERROR\n");
            break;

        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGI(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            // If user_data buffer is configured, copy the response into the buffer

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
                            ESP_LOGE(TAG, "Failed to allocate memory for output buffer");
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
            ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH");
            if (output_buffer != NULL) {
                process_incoming_data(output_buffer, output_len);           
            }
            if (output_buffer != NULL) {
                free(output_buffer);
                output_buffer = NULL;
            }
            output_len = 0;
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
            int mbedtls_err = 0;
            esp_err_t err = esp_tls_get_and_clear_last_error((esp_tls_error_handle_t)evt->data, &mbedtls_err, NULL);
            if (err != 0) {
                ESP_LOGI(TAG, "Last esp error code: 0x%x", err);
                ESP_LOGI(TAG, "Last mbedtls failure: 0x%x", mbedtls_err);
            }
            if (output_buffer != NULL) {
                free(output_buffer);
                output_buffer = NULL;
            }
            output_len = 0;
            break;

        default:
            PRINTFC_MAIN("HTTP_EVENT = %d\n Look it up! \n", evt->event_id);
            break;          
    }
    return ESP_OK;
}

void process_incoming_data(char *output_buffer, int output_len){
     cJSON *json = cJSON_Parse(output_buffer);
    if (json != NULL) {
        if (cJSON_HasObjectItem(json, "id")) {
            cJSON *id = cJSON_GetObjectItem(json, "id");
            if (id != NULL && cJSON_IsString(id)) {
                PRINTFC_MAIN("Player ID: %s\n", id->valuestring);
                memcpy(playerID, id->valuestring, MIN(output_len, sizeof(playerID) - 1));
                PRINTFC_MAIN("Player ID recieved: %s\n", playerID);
                xEventGroupSetBits(wifi_event_group, GOT_PLAYER_ID_BIT);
            } else {
                PRINTFC_MAIN("Invalid 'id' in JSON\n");
            }
        } else {
            PRINTFC_MAIN("JSON does not contain 'id'\n");
            PRINTFC_MAIN("JSON: %s\n", output_buffer);
        }
        cJSON_Delete(json); // Clean up parsed JSON
        return;
    }

      char *cert_start = strstr((char*)output_buffer, "-----BEGIN CERTIFICATE-----");

       int err = save_certificate(cert_start, output_len);
       if (err == 0) {
            xEventGroupSetBits(wifi_event_group, GOT_CERTIFICATE_BIT);
            PRINTFC_MAIN("Certificate saved \n");
        } else {
            PRINTFC_MAIN("Error with certificate \n");
        }
        // You can handle the certificate data here
        return;
    }

int save_certificate(char *cert_start, int output_len){
    
        char *cert_end = strstr(cert_start, "-----END CERTIFICATE-----");
        if (cert_end) {
            cert_end += strlen("-----END CERTIFICATE-----"); // Move to the end of the certificate

        int cert_len = cert_end - cert_start;
        if (cert_len < sizeof(signed_certificate)) {
            memset(signed_certificate, 0, sizeof(signed_certificate));
            strncpy(signed_certificate, cert_start, cert_len);
            signed_certificate[cert_len] = '\0'; 
            return 0;
        }
         
    }
    return 1; 
}

/*

 PRINTFC_CLIENT("Raw output buffer: %.*s", output_len, output_buffer);
    char *player_id_start = strstr((char*)output_buffer, "{\"id");
                    if (player_id_start) {
                        player_id_start += strlen("{\"id\": \"");
                        
                        char *player_id_end = strstr(player_id_start, "\"}");
                        if (player_id_end) {
                            int player_id_len = player_id_end - player_id_start;
                            if (player_id_len < sizeof(playerID)) {
                                memset(playerID, 0, sizeof(playerID));
                                strncpy(playerID, player_id_start, player_id_len);
                                playerID[player_id_len] = '\0'; // Ensure null-termination
                                PRINTFC_CLIENT("Player ID: %s", playerID);
                                xEventGroupSetBits(wifi_event_group, GOT_PLAYER_ID_BIT);
                                return;
                            } else {
                                PRINTFC_CLIENT("Error: Player ID buffer too small");
                            }
                        } else {
                            PRINTFC_CLIENT("Error: Missing end quote for player ID");
                        }
                    } else {
                        PRINTFC_CLIENT("Not a player ID response");

                    }
*/