#include "mqtt_handler.h"
#include "printer_helper.h"
#include "certs.h"

#define BROKER_IP "172.20.80.1"
#define MQTT_BROKER_URI "mqtts://" BROKER_IP ":8883"

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
        msg_id = esp_mqtt_client_subscribe(client, "/spelare/qos0", 0);
        PRINTFC_MQTT("sent subscribe successful, msg_id=%d", msg_id);

        msg_id = esp_mqtt_client_subscribe(client, "/spelare/qos1", 1);
        PRINTFC_MQTT("sent subscribe successful, msg_id=%d", msg_id);

        msg_id = esp_mqtt_client_unsubscribe(client, "/spelare/qos1");
        PRINTFC_MQTT("sent unsubscribe successful, msg_id=%d", msg_id);
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
    PRINTFC_MQTT("MQTT app starting");
    const esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = MQTT_BROKER_URI,
        .broker.verification.certificate = (const char *)server_cert_pem_start,
        .credentials = {
            .authentication = {
            .certificate = (const char *)client_cert_pem_start,
            .key = (const char *)client_key_pem_start,
            },
        }
    };
        
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
    return client;
}
