#include "serial.h"
#include "driver/uart.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "printer_helper.h"
#include "string.h"
#include "shared_resources.h"
<<<<<<< HEAD
#include "mqtt_client.h"

=======
#include "stdio.h"
>>>>>>> patriks

#define UART_NUM UART_NUM_0
#define BUF_SIZE 1024
#define TX_PIN 16
#define RX_PIN 17

#define MAX_MSG_LEN 128

void serial_task(void *pvParameters)
{
    PRINTFC_SERIAL("Serial task started");

    // UART configuration
    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 122,
    };

    uint8_t data[128];
    char input_buffer[128] = {0};
    int buffer_index = 0;

    // Install UART driver
    ESP_ERROR_CHECK(uart_param_config(UART_NUM, &uart_config));
    uart_set_pin(UART_NUM, TX_PIN, RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_NUM, BUF_SIZE * 2, BUF_SIZE * 2, 20, &serial_msg_queue, 0);

    PRINTFC_SERIAL("UART driver installed");

    while (1)
    {
        char* input_string = read_uart_data(data, input_buffer, &buffer_index);
        if (input_string != NULL) {
            // Process the received string here
            printf("\n Processed: %s\n", input_string);

<<<<<<< HEAD
                    // Echo the data back
                    uart_write_bytes(UART_NUM, (const char *)data, event.size);
                    
                    int len = uart_read_bytes(UART_NUM, data, BUF_SIZE, 20 / portTICK_PERIOD_MS);
                    if (event.size > 0) {
                        data[len] = '\0';  // Null terminate the string

                        // Process commands from serial input
                        PRINTFC_SERIAL("Received command: %s", data);
                        
                        // Handle choice commands
                        if (strstr((char *)data, "val ok")) {
                            PRINTFC_SERIAL("Choice: OK");
                            esp_mqtt_client_publish(mqtt_client, TOPIC_PLAYER_Uplink, "{\"val\": \"ok\"}", 0, 1, 0);
                        } else if (strstr((char *)data, "val neka")) {
                            PRINTFC_SERIAL("Choice: NEKA");
                            esp_mqtt_client_publish(mqtt_client, TOPIC_PLAYER_Uplink, "{\"val\": \"neka\"}", 0, 1, 0);
                        } else if (strstr((char *)data, "val lyckas")) {
                            PRINTFC_SERIAL("Choice: LYCKAS");
                            esp_mqtt_client_publish(mqtt_client, TOPIC_PLAYER_Uplink, "{\"val\": \"lyckas\"}", 0, 1, 0);
                        } else if (strstr((char *)data, "val sabotage")) {
                            PRINTFC_SERIAL("Choice: SABOTAGE");
                            esp_mqtt_client_publish(mqtt_client, TOPIC_PLAYER_Uplink, "{\"val\": \"sabotage\"}", 0, 1, 0);
                        } else if (strstr((char *)data, "val starta")) {
                            PRINTFC_SERIAL("Choice: STARTA SPEL");
                            esp_mqtt_client_publish(mqtt_client, TOPIC_PLAYER_Uplink, "{\"val\": \"starta\"}", 0, 1, 0);
                        } else if (strstr((char *)data, ":")) {
                            PRINTFC_SERIAL("Chat message sent.");
                            esp_mqtt_client_publish(mqtt_client, TOPIC_TORGET, (char *)data, 0, 1, 0);
                        } else {
                            PRINTFC_SERIAL("Unknown choice command.");
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
=======
>>>>>>> patriks
        }
        vTaskDelay(pdMS_TO_TICKS(50));  // Optional delay
                   
    }
    vTaskDelete(NULL);
}

<<<<<<< HEAD
void create_topic(char *topic, size_t topic_size, const char *base_topic, char* player_id) {
    // Format the topic string based on the base topic and player ID
    snprintf(topic, topic_size, base_topic, player_id);
=======
char* read_uart_data(uint8_t* data, char* input_buffer, int* buffer_index) {

    int len = uart_read_bytes(UART_NUM_0, data, 128, pdMS_TO_TICKS(100));  //nått kan va knas här med pdms
    if (len > 0) {
        for (int i = 0; i < len; i++) {
            if (data[i] == '\n' || data[i] == '\r') {  // End of input
                input_buffer[*buffer_index] = '\0';   // Null-terminate
                *buffer_index = 0;                   // Reset index
                return input_buffer;                // Return the string
            } else {
                if (*buffer_index < 127) {
                    input_buffer[*buffer_index] = data[i];
                    printf("%c", data[i]);
                    fflush(stdout);
                    (*buffer_index)++;
                } else {
                    // Buffer overflow
                    printf("Error: Input buffer overflow. Resetting.\n");
                    fflush(stdout);
                    *buffer_index = 0;               // Reset index
                }
            }
        }
    }
    return NULL;  // No complete line received yet
>>>>>>> patriks
}