#include "serial.h"
#include "driver/uart.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "printer_helper.h"
#include "string.h"
#include "shared_resources.h"
#include "mqtt_client.h"


#define UART_NUM UART_NUM_0
#define BUF_SIZE 1024
#define TX_PIN 16
#define RX_PIN 17

#define MAX_MSG_LEN 128

void serial_task(void *pvParameters)
{
    PRINTFC_SERIAL("Serial task started");

    uint8_t data[BUF_SIZE];
    uart_event_t event;

    // UART configuration
    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 122,
    };

    // Install UART driver
    ESP_ERROR_CHECK(uart_param_config(UART_NUM, &uart_config));
    uart_set_pin(UART_NUM, TX_PIN, RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_NUM, BUF_SIZE * 2, BUF_SIZE * 2, 20, &serial_msg_queue, 0);

    PRINTFC_SERIAL("UART driver installed");

    while (1)
    {
        // Wait for UART event
        if (xQueueReceive(serial_msg_queue, (void *)&event, portMAX_DELAY) == pdTRUE)
        {
            switch (event.type)
            {
                case UART_DATA:
                    memset(data, 0, sizeof(data));
                    uart_read_bytes(UART_NUM, data, event.size, pdTICKS_TO_MS(500));
                    data[event.size] = '\0'; // Ensure null-termination
                    PRINTFC_SERIAL("Received %d bytes", event.size);
                    PRINTFC_SERIAL("Data: %s", data);

                    // Echo the data back
                    uart_write_bytes(UART_NUM, (const char *)data, event.size);
                    
                    int len = uart_read_bytes(UART_NUM, data, BUF_SIZE, 20 / portTICK_PERIOD_MS);
                    if (len > 0) {
                        data[len] = '\0';  // Null terminate the string

                        // Process commands from serial input
                        PRINTFC_SERIAL("Received command: %s", data);
                        
                        // Handle "start round" command
                        if (strstr((char *)data, "start round")) {
                            // Send new round message to MQTT
                            char msg[MAX_MSG_LEN];
                            snprintf(msg, sizeof(msg), "{\"typ\": \"ny runda\", \"data\": [1, 2, 3]}");  // List of players who accepted the leader
                            esp_mqtt_client_publish(mqtt_client, TOPIC_MYNDIGHETEN, msg, 0, 1, 0);
                            PRINTFC_SERIAL("New round started!");
                        }
                        // Handle "choose leader" command
                        else if (strstr((char *)data, "choose leader")) {
                            // Send leader choice message to MQTT
                            char msg[MAX_MSG_LEN];
                            snprintf(msg, sizeof(msg), "{\"typ\": \"val av ledare\", \"data\": 3}");  // Leader chosen with ID 3
                            esp_mqtt_client_publish(mqtt_client, TOPIC_MYNDIGHETEN, msg, 0, 1, 0);
                            PRINTFC_SERIAL("Leader chosen!");
                        }
                        // Handle "ok" vote for leader
                        else if (strstr((char *)data, "vote ok")) {
                            // Send vote to leader's uplink topic
                            char msg[MAX_MSG_LEN];
                            snprintf(msg, sizeof(msg), "{\"val\": \"ok\"}");
                            char topic[MAX_MSG_LEN];
                            create_topic(topic, sizeof(topic), TOPIC_PLAYER_Uplink, shorter_id);  // Send to player's uplink topic
                            esp_mqtt_client_publish(mqtt_client, topic, msg, 0, 1, 0);
                            PRINTFC_SERIAL("Vote OK sent to leader!");
                        }
                        // Handle "neka" vote for leader
                        else if (strstr((char *)data, "vote neka")) {
                            // Send vote to leader's uplink topic
                            char msg[MAX_MSG_LEN];
                            snprintf(msg, sizeof(msg), "{\"val\": \"neka\"}");
                            char topic[MAX_MSG_LEN];
                            create_topic(topic, sizeof(topic), TOPIC_PLAYER_Uplink, shorter_id);  // Send to player's uplink topic
                            esp_mqtt_client_publish(mqtt_client, topic, msg, 0, 1, 0);
                            PRINTFC_SERIAL("Vote NEKA sent to leader!");
                        }
                        // Handle "start mission" command
                        else if (strstr((char *)data, "start mission")) {
                            // Send start mission message to MQTT
                            char msg[MAX_MSG_LEN];
                            snprintf(msg, sizeof(msg), "{\"typ\": \"ny runda\", \"data\": [1, 2, 3]}");  // Players who agreed to start the mission
                            esp_mqtt_client_publish(mqtt_client, TOPIC_MYNDIGHETEN, msg, 0, 1, 0);
                            PRINTFC_SERIAL("Mission started!");
                        }
                        // Handle "sabotage" command
                        else if (strstr((char *)data, "sabotage")) {
                            // Send sabotage action to MQTT (during the mission)
                            char msg[MAX_MSG_LEN];
                            snprintf(msg, sizeof(msg), "{\"val\": \"sabotera\"}");
                            char topic[MAX_MSG_LEN];
                            create_topic(topic, sizeof(topic), TOPIC_PLAYER_Uplink, shorter_id);  // Send to player's uplink topic
                            esp_mqtt_client_publish(mqtt_client, topic, msg, 0, 1, 0);
                            PRINTFC_SERIAL("Sabotage action sent!");
                        }
                        // Handle "mission success" command
                        else if (strstr((char *)data, "mission success")) {
                            // Send mission success message to MQTT
                            char msg[MAX_MSG_LEN];
                            snprintf(msg, sizeof(msg), "{\"typ\": \"uppdrag lyckades\", \"data\": %s}", shorter_id);  // Mission leader ID
                            esp_mqtt_client_publish(mqtt_client, TOPIC_MYNDIGHETEN, msg, 0, 1, 0);
                            PRINTFC_SERIAL("Mission succeeded!");
                        }
                        // Handle "mission sabotaged" command
                        else if (strstr((char *)data, "mission sabotaged")) {
                            // Send sabotage notification to MQTT
                            char msg[MAX_MSG_LEN];
                            snprintf(msg, sizeof(msg), "{\"typ\": \"uppdrag saboterat\", \"data\": %s}", shorter_id);  // Player ID of sabotager
                            esp_mqtt_client_publish(mqtt_client, TOPIC_MYNDIGHETEN, msg, 0, 1, 0);
                            PRINTFC_SERIAL("Mission sabotaged!");
                        }
                        // Handle "kick leader" command
                        else if (strstr((char *)data, "kick leader")) {
                            // Send "kick" vote to the leader's downlink topic
                            char msg[MAX_MSG_LEN];
                            snprintf(msg, sizeof(msg), "{\"val\": \"ok\"}");  // Vote to kick the leader
                            char topic[MAX_MSG_LEN];
                            create_topic(topic, sizeof(topic), TOPIC_PLAYER_Downlink, shorter_id);  // Send to player's downlink topic
                            esp_mqtt_client_publish(mqtt_client, topic, msg, 0, 1, 0);
                            PRINTFC_SERIAL("Kick vote sent to leader!");
                        }
                        // Handle "tillit" (trust) command
                        else if (strstr((char *)data, "tillit")) {
                            // Send trust update to the server
                            char msg[MAX_MSG_LEN];
                            snprintf(msg, sizeof(msg), "{\"typ\": \"tillit\", \"data\": %d}", 5);  // Example: 5 trust points
                            esp_mqtt_client_publish(mqtt_client, TOPIC_MYNDIGHETEN, msg, 0, 1, 0);
                            PRINTFC_SERIAL("Trust level updated!");
                        }
                        // Handle "successful missions" command
                        else if (strstr((char *)data, "successful missions")) {
                            // Send count of successful missions
                            char msg[MAX_MSG_LEN];
                            snprintf(msg, sizeof(msg), "{\"typ\": \"lyckade uppdrag\", \"data\": %d}", 3);  // Example: 3 successful missions
                            esp_mqtt_client_publish(mqtt_client, TOPIC_MYNDIGHETEN, msg, 0, 1, 0);
                            PRINTFC_SERIAL("Successfully completed missions updated!");
                        }
                        else {
                            PRINTFC_SERIAL("Unknown command.");
                        }
                    }
                    break;

                case UART_FIFO_OVF:
                    PRINTFC_SERIAL("UART FIFO overflow");
                    uart_flush_input(UART_NUM);
                    xQueueReset(serial_msg_queue);
                    break;

                case UART_BUFFER_FULL:
                    PRINTFC_SERIAL("UART buffer full");
                    uart_flush_input(UART_NUM);
                    xQueueReset(serial_msg_queue);
                    break;

                case UART_PARITY_ERR:
                    PRINTFC_SERIAL("UART parity error");
                    break;

                case UART_FRAME_ERR:
                    PRINTFC_SERIAL("UART frame error");
                    break;

                default:
                    PRINTFC_SERIAL("UART event type: %d", event.type);
                    break;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1000)); // Adjust the delay as needed
    }
    vTaskDelete(NULL);
}

void create_topic(char *topic, size_t topic_size, const char *base_topic, char* player_id) {
    // Format the topic string based on the base topic and player ID
    snprintf(topic, topic_size, base_topic, player_id);
}