#include "mqtt_handler.h"
#include "mqtt_client.h" // Add this line to include the header file that defines MQTT_PROTOCOL_V311
#include "sdkconfig.h"

static const char *TAG = "MQTT_HANDLER";

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        PRINTFC_MQTT("Last error %s: 0x%x", message, error_code);
    }
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
   // PRINTFC_MQTT("Event dispatched from event loop base=%s, event_id=%" PRIi32, base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
    //    PRINTFC_MQTT("MQTT_EVENT_CONNECTED");
        xEventGroupSetBits(wifi_event_group, MQTT_CLIEN_CONNECTED_BIT);

        mqtt_subscribe(client);
      //  esp_mqtt_client_publish(client, "/torget", test_msg, strlen(test_msg), 1, 0);
        break;
    case MQTT_EVENT_DISCONNECTED:
     //   PRINTFC_MQTT("MQTT_EVENT_DISCONNECTED");
        break;
    case MQTT_EVENT_SUBSCRIBED:
     //   PRINTFC_MQTT("MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);     
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
     //   PRINTFC_MQTT("MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        PRINTFC_MQTT("MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
       PRINTFC_MQTT("MQTT_EVENT_DATA");
       PRINTFC_MQTT("TOPIC= %.*s\r\n", event->topic_len, event->topic);
       PRINTFC_MQTT("DATA=m %.*s\r\n", event->data_len, event->data);
        break;
   case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            if (event->error_handle->error_type == MQTT_ERROR_TYPE_ESP_TLS) {
                ESP_LOGI(TAG, "Last error code reported from esp-tls: 0x%x", event->error_handle->esp_tls_last_esp_err);
                ESP_LOGI(TAG, "Last tls stack error number: 0x%x", event->error_handle->esp_tls_stack_err);
                ESP_LOGI(TAG, "Last captured errno : %d (%s)", event->error_handle->esp_transport_sock_errno, strerror(event->error_handle->esp_transport_sock_errno));
            } else if (event->error_handle->error_type == MQTT_ERROR_TYPE_CONNECTION_REFUSED) {
                ESP_LOGI(TAG, "Connection refused error: 0x%x", event->error_handle->connect_return_code);
            } else {
                ESP_LOGW(TAG, "Unknown error type: 0x%x", event->error_handle->error_type);
            }
            break;
    default:
        PRINTFC_MQTT("Other event id:%d", event->event_id);
        break;
    }
}

esp_mqtt_client_handle_t mqtt_app_start()
{
    xEventGroupWaitBits(wifi_event_group, GOT_CERTIFICATE_BIT, pdFALSE, pdTRUE, portMAX_DELAY);
    
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
        
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    if (client == NULL) {
        PRINTFC_MQTT("Failed to initialize MQTT client");
        return NULL;
    }

    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);



    esp_err_t err = esp_mqtt_client_start(client);
    if (err != ESP_OK) {
        PRINTFC_MQTT("Failed to start MQTT client: %s", esp_err_to_name(err));
        return NULL;
    }

    return client;
}
void mqtt_subscribe(esp_mqtt_client_handle_t client)
{
    
    snprintf(topic_player_uplink, sizeof(topic_player_uplink), "/spelare/%s/uplink", playerID);
    PRINTFC_MQTT("Subscribing to /torget");
    int msg_id = esp_mqtt_client_subscribe(client, "/torget", 0);
    PRINTFC_MQTT("sent subscribe successful, msg_id=%d", msg_id);
     msg_id = esp_mqtt_client_subscribe(client, "/myndigheten", 0);
     msg_id = esp_mqtt_client_subscribe(client, topic_player_uplink, 0);
     
}

void mqtt_torget(char* data, esp_mqtt_client_handle_t client)
{
    char message[512];
    snprintf(message, sizeof(message), "{\"id\": \"%s\", \"data\": \"%s\"}", playerID, data);
    esp_mqtt_client_publish(client, "/torget", message, 0, 1, 0);
}
void mqtt_uplink(char* data, esp_mqtt_client_handle_t client)
{
    char message[512];
    if (strstr((char*)data, "/val ok")){
        esp_mqtt_client_publish(mqtt_client, topic_player_uplink, "{\"val\": \"ok\"}", 0, 1, 0);
    }
    
   
}

/*

    if (strstr((char *)data, "/v ok")) {
                            PRINTFC_SERIAL("Choice: OK");
                            esp_mqtt_client_publish(mqtt_client, topic_player_uplink, "{\"val\": \"ok\"}", 0, 1, 0);
                        } else if (strstr((char *)data, "/v neka")) {
                            PRINTFC_SERIAL("Choice: NEKA");
                            esp_mqtt_client_publish(mqtt_client, topic_player_uplink, "{\"val\": \"neka\"}", 0, 1, 0);
                        } else if (strstr((char *)data, "/v lyckas")) {
                            PRINTFC_SERIAL("Choice: LYCKAS");
                            esp_mqtt_client_publish(mqtt_client, topic_player_uplink, "{\"val\": \"lyckas\"}", 0, 1, 0);
                        } else if (strstr((char *)data, "/v sabotage")) {
                            PRINTFC_SERIAL("Choice: SABOTAGE");
                            esp_mqtt_client_publish(mqtt_client, topic_player_uplink, "{\"val\": \"sabotage\"}", 0, 1, 0);
                        } else if (strstr((char *)data, "/v starta")) {
                            PRINTFC_SERIAL("Choice: STARTA SPEL");
                            esp_mqtt_client_publish(mqtt_client, topic_player_uplink, "{\"val\": \"starta\"}", 0, 1, 0);
                        } else if (data[0] == ':') {
                            PRINTFC_SERIAL("Chat message sent: %s", (data + 1));
                            char chat_message[512];
                            snprintf(chat_message, sizeof(chat_message), "{\"id\": \"%s\", \"data\": \"%s\"}", shorter_id, (data + 1));
                            esp_mqtt_client_publish(mqtt_client, TOPIC_TORGET, (char *)(chat_message), 0, 1, 0);
                        } else if (strstr((char *)data, "/r reg")) {
                            PRINTFC_SERIAL("Choice: REG_PLAYER");
                            xEventGroupSetBits(wifi_event_group, BIT3);
                        } else if (strstr((char *)data, "/r csr")) {
                            PRINTFC_SERIAL("Choice: SEND_CSR");
                            xEventGroupSetBits(wifi_event_group, BIT4);
                        } else if (strstr((char *)data , "/r start")) {
                            PRINTFC_SERIAL("Choice: START_GAME");
                            xEventGroupSetBits(wifi_event_group, BIT5);
                        } else {
                            PRINTFC_SERIAL("Unknown choice command.");
                        }
                    }


*/