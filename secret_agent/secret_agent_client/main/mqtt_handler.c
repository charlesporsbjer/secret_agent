#include "mqtt_handler.h"
#include "mqtt_client.h" // Add this line to include the header file that defines MQTT_PROTOCOL_V311
#include "sdkconfig.h"

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        PRINTFC_MQTT("Last error %s: 0x%x", message, error_code);
    }
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    PRINTFC_MQTT("Event dispatched from event loop base=%s, event_id=%" PRIi32, base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        PRINTFC_MQTT("MQTT_EVENT_CONNECTED");
        xEventGroupSetBits(wifi_event_group, MQTT_CLIEN_CONNECTED_BIT);

        // fyll ut

        break;
    case MQTT_EVENT_DISCONNECTED:
        PRINTFC_MQTT("MQTT_EVENT_DISCONNECTED");
        break;
    case MQTT_EVENT_SUBSCRIBED:
        PRINTFC_MQTT("MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);

        //// fyll ut
       
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        PRINTFC_MQTT("MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        PRINTFC_MQTT("MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        PRINTFC_MQTT("MQTT_EVENT_DATA");
        break;
    case MQTT_EVENT_ERROR:
        PRINTFC_MQTT("MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            // log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            // log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            // log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
            // PRINTFC_MQTT("Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
        }
        break;
    default:
        PRINTFC_MQTT("Other event id:%d", event->event_id);
        break;
    }
}

esp_mqtt_client_handle_t mqtt_app_start()
{
    xEventGroupWaitBits(wifi_event_group,GAME_STARTED_BIT, pdFALSE, pdTRUE, portMAX_DELAY);
    esp_log_level_set("esp-tls", ESP_LOG_DEBUG);
    esp_log_level_set("mbedtls", ESP_LOG_DEBUG);
    esp_log_level_set("TRANSPORT", ESP_LOG_DEBUG);

    PRINTFC_MQTT("MQTT app starting");
  //  PRINTFC_MQTT("key_pem after type conversion: %s", (const char *)key_pem);
 //   PRINTFC_MQTT("signed_certificate after type conversion: %s", (const char *)signed_certificate);
    PRINTFC_MQTT("Broker address: %s", MQTT_BROKER_URI);
    strncpy(shorter_id, playerID, 32);

    esp_mqtt_client_config_t mqtt_cfg = {
        .broker = {
            .address.uri = MQTT_BROKER_URI,
            .verification = {
                .certificate = (const char*)ca_cert_pem_start,
                .skip_cert_common_name_check = true,
                
            },
    
            
        },

        .credentials = {
            
            .authentication = {
                .certificate = (const char*)signed_certificate,
                .key = (const char *)key_pem,
            },
            .client_id = playerID,
            
        },
        .network.timeout_ms = 10000, // Increase timeout to 10 seconds
    };
    mqtt_cfg.session.keepalive = 60;
        
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    if (client == NULL) {
        PRINTFC_MQTT("Failed to initialize MQTT client");
        return NULL;
    }

    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
 //   esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);

  //  esp_mqtt_client_subscribe(client, "/torget", 0);

    esp_err_t err = esp_mqtt_client_start(client);
    if (err != ESP_OK) {
        PRINTFC_MQTT("Failed to start MQTT client: %s", esp_err_to_name(err));
        return NULL;
    }

    return client;
}

/*


esp_mqtt_client_handle_t mqtt_app_start()
{
    xEventGroupWaitBits(wifi_event_group, BIT0 | BIT1 | BIT2, pdFALSE, pdTRUE, portMAX_DELAY);
    PRINTFC_MQTT("MQTT app starting");
#ifdef DEBUG_MODE
    esp_log_level_set("esp-tls", ESP_LOG_DEBUG);
    esp_log_level_set("mbedtls", ESP_LOG_DEBUG);
    PRINTFC_MQTT("key_pem after type conversion: %s", (const char *)key_pem);
#endif
    PRINTFC_MQTT("Broker address: %s", MQTT_BROKER_URI);
    strncpy(shorter_id, player_id, 32);
    snprintf(topic_player_uplink, sizeof(topic_player_uplink), "/spelare/%s/uplink", shorter_id);

    const esp_mqtt_client_config_t mqtt_cfg = {
        .broker = {
            .address.uri = MQTT_BROKER_URI,
            .verification = {
                .certificate = (const char*)ca_server_copy,
                .skip_cert_common_name_check = true,
            },
        },

        .credentials = {
            .authentication = {
                .certificate = (const char*)signed_certificate,
                .key = (const char *)key_pem,
            },
            .client_id = player_id,
        },
        .network.timeout_ms = 10000, // Increase timeout to 10 seconds
    };
        
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    if (client == NULL) {
        PRINTFC_MQTT("Failed to initialize MQTT client");
        return NULL;
    }

    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);

    esp_err_t err = esp_mqtt_client_start(client);
    if (err != ESP_OK) {
        PRINTFC_MQTT("Failed to start MQTT client: %s", esp_err_to_name(err));
        return NULL;
    }

   


*/