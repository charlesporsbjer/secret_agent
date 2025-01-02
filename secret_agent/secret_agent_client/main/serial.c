#include "serial.h"
#include "driver/uart.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "printer_helper.h"
#include "string.h"
#include "shared_resources.h"

#define UART_NUM UART_NUM_0
#define BUF_SIZE 1024
#define TX_PIN 16
#define RX_PIN 17

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