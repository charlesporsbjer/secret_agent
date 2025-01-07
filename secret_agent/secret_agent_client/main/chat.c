#include "chat.h"
#include "serial.h"
#include "mqtt_handler.h"
#include "printer_helper.h"
#include "client.h"
#include "shared_resources.h"

#define BUF_SIZE 256

void chat_task(void* mqtt_client_p)
{
    esp_mqtt_client_handle_t mqtt_client = (esp_mqtt_client_handle_t)mqtt_client_p;
    PRINTFC_CHAT("Chat task is starting");

    char serial_msg[BUF_SIZE];
    char mqtt_msg[BUF_SIZE];

    while (1)
    {
        // Check for MQTT messages
        if (xSemaphoreTake(xSemaphore_serial, portMAX_DELAY) == pdTRUE)
        {   
            // Check for serial messages
            if (xQueueReceive(serial_msg_queue, &serial_msg, portMAX_DELAY))
            {
                if (serial_msg[0] == ':' && (serial_msg[1] == 's' ||serial_msg[1] == 'S') && serial_msg[2] == ' ')
                {
                    if (xSemaphoreTake(xSemaphore_mqtt_client, portMAX_DELAY) == pdTRUE)
                    {
                        PRINTFC_SERIAL("Serial: %s", serial_msg + 3);
                        // Publish the message to the chat topic
                        mqtt_publish(mqtt_client, "/torget", serial_msg + 3);
                    }
                    PRINTFC_CHAT("Chat: %s", serial_msg + 3);
                }
            }
            xSemaphoreGive(xSemaphore_mqtt_client);
        }

        if (xSemaphoreTake(xSemaphore_mqtt_evt, portMAX_DELAY) == pdTRUE)
        {
            if (xQueueReceive(mqtt_event_queue, &mqtt_msg, 0))
            {
                // Assuming the topic and payload are concatenated with a delimiter, e.g., ":"
                char *topic = strtok(mqtt_msg, ":");
                char *payload = strtok(NULL, ":");

                if (topic && payload)
                {
                    if (strcmp(topic, "/torget") == 0)
                    {
                        PRINTFC_TORGET("%s", payload);
                    }
                    else if (strcmp(topic, "/myndigheten") == 0)
                    {
                        PRINTFC_MYNDIGHETEN("%s", payload);
                    }
                }
            }
            xSemaphoreGive(xSemaphore_mqtt_evt);
        }
        vTaskDelay(pdMS_TO_TICKS(1000)); // Adjust the delay as needed
    }
    vTaskDelete(NULL);
}