#include "mqtt_handler.h"
#include "printer_helper.h"
#include "certs.h"
#include "shared_resources.h"
#include "esp_log.h"
#include "esp_event.h"

#define MAX_PLAYER_ID_LEN 32
#define MAX_TOPIC_LEN (17 + MAX_PLAYER_ID_LEN + 1) // 17 for base string + max player_id length + null terminator

// Zainab "172.16.217.104"
// Me 172.16.217.226
#define BROKER_IP "172.16.217.226"
#define MQTT_BROKER_URI "mqtts://" BROKER_IP ":8884" // 8883 or 8884

char shorter_id[32] = {0};

#define MAX_MSG_LEN 128

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
        msg_id = esp_mqtt_client_subscribe(client, "/torget", 0);
        PRINTFC_MQTT("Subscribed to /torget, msg_id=%d", msg_id);
        msg_id = esp_mqtt_client_subscribe(client, "/myndigheten", 0);
        PRINTFC_MQTT("Subscribed to /myndigheten, msg_id=%d", msg_id);
        // Subscribe to player-specific topics
        char uplink_topic[MAX_TOPIC_LEN];
        char downlink_topic[MAX_TOPIC_LEN];
        snprintf(uplink_topic, sizeof(uplink_topic), "/spelare/%s/uplink", shorter_id);
        snprintf(downlink_topic, sizeof(downlink_topic), "/spelare/%s/downlink", shorter_id);
        msg_id = esp_mqtt_client_subscribe(client, uplink_topic, 0);
        PRINTFC_MQTT("Subscribed to %s, msg_id=%d", uplink_topic, msg_id);
        msg_id = esp_mqtt_client_subscribe(client, downlink_topic, 0);
        PRINTFC_MQTT("Subscribed to %s, msg_id=%d", downlink_topic, msg_id);
        break;
    case MQTT_EVENT_DISCONNECTED:
        PRINTFC_MQTT("MQTT_EVENT_DISCONNECTED");
        break;
    case MQTT_EVENT_SUBSCRIBED:
        PRINTFC_MQTT("MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        msg_id = esp_mqtt_client_publish(client, "/spelare/qos0", "data", 0, 0, 0);
        PRINTFC_MQTT("sent publish successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        PRINTFC_MQTT("MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        PRINTFC_MQTT("MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        PRINTFC_MQTT("MQTT_EVENT_DATA");
        PRINTFC_MQTT("TOPIC=%.*s\r", event->topic_len, event->topic);
        PRINTFC_MQTT("DATA=%.*s\r", event->data_len, event->data);
        
        mqtt_message_handler(event_data);

        break;
    case MQTT_EVENT_ERROR:
        PRINTFC_MQTT("MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
            PRINTFC_MQTT("Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
        }
        break;
    default:
        PRINTFC_MQTT("Other event id:%d", event->event_id);
        break;
    }
}

// Initialize MQTT Client
esp_mqtt_client_handle_t mqtt_app_start()
{

    esp_log_level_set("esp-tls", ESP_LOG_DEBUG);
    esp_log_level_set("mbedtls", ESP_LOG_DEBUG);
    
    xEventGroupWaitBits(wifi_event_group, BIT0 | BIT1 | BIT2, pdFALSE, pdTRUE, portMAX_DELAY);
    PRINTFC_MQTT("MQTT app starting");
    PRINTFC_MQTT("key_pem after type conversion: %s", (const char *)key_pem);
    PRINTFC_MQTT("Broker address: %s", MQTT_BROKER_URI);
    strncpy(shorter_id, player_id, 32);

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

    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);

    esp_err_t err = esp_mqtt_client_start(client);
    if (err != ESP_OK) {
        PRINTFC_MQTT("Failed to start MQTT client: %s", esp_err_to_name(err));
        return NULL;
    }

    return client;
}

void mqtt_message_handler(void *event_data) {
    esp_mqtt_event_handle_t event = event_data;
    char topic[MAX_TOPIC_LEN];
    char msg[MAX_MSG_LEN];
    int player_id;
    int leader_id;

    // Handle messages based on the topic
    if (strcmp(event->topic, "/spelare/") == 0) {
        // Example: Player sends "ok" or "neka" to vote for leader
        if (strstr(event->data, "ok") != NULL) {
            PRINTFC_MQTT("Player voted 'ok' for leader.");
            // Update game state based on vote (e.g., add player to accepted leader list)
            // Send update to server if needed
        } else if (strstr(event->data, "neka") != NULL) {
            PRINTFC_MQTT("Player voted 'neka' for leader.");
            // Update game state based on vote (e.g., player rejects leader)
            // Send update to server if needed
        }
    }
    else if (strcmp(event->topic, "/spelare/+/downlink") == 0) {
        // Example: Server sends messages such as mission sabotage or success
        if (strstr(event->data, "uppdrag lyckades") != NULL) {
            PRINTFC_MQTT("Mission succeeded, player %s led the mission.", event->data);
            // Handle mission success logic (e.g., update state, notify players)
        } else if (strstr(event->data, "uppdrag saboterat") != NULL) {
            PRINTFC_MQTT("Mission was sabotaged by player %s.", event->data);
            // Handle sabotage logic (e.g., update state, notify players)
        }
    }
    else if (strcmp(event->topic, "/myndigheten") == 0) {
        // Server broadcasts updates to all players
        if (strstr(event->data, "ny runda") != NULL) {
            PRINTFC_MQTT("New round has started, player list: %s", event->data);
            // Handle the start of a new round (e.g., update game state)
        } else if (strstr(event->data, "val av ledare") != NULL) {
            leader_id = atoi(event->data);  // Extract leader ID from message
            PRINTFC_MQTT("New leader chosen: Player %d", leader_id);
            // Notify all players to vote for the leader
        } else if (strstr(event->data, "sparka spelare") != NULL) {
            player_id = atoi(event->data);  // Extract player ID from message
            PRINTFC_MQTT("Player %d has been kicked from the game.", player_id);
            // Handle player removal from game
        }
    }
    else {
        PRINTFC_MQTT("Unhandled topic: %s", event->topic);
    }
}