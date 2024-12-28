#include "serial.h"
#include "driver/uart.h"
#include "freertos/task.h"
#include "printer_helper.h"
#include "string.h"

#define UART_NUM UART_NUM_1
#define BUF_SIZE 1024
#define TX_PIN 17
#define RX_PIN 16

void serial_task(void *pvParameters)
{

    uint8_t data[BUF_SIZE];
    uint8_t buffer[BUF_SIZE];
    uart_event_t event;

    // UART configuration
    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };

    // Install UART driver
    uart_param_config(UART_NUM, &uart_config);
    uart_set_pin(UART_NUM, TX_PIN, RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_NUM, BUF_SIZE * 2, BUF_SIZE * 2, 20, &serial_msg_queue, 0);

    while (1)
    {
        if (xSemaphoreTake(xSemaphore_serial, portMAX_DELAY) == pdTRUE) 
        {
            if (xQueueReceive(serial_msg_queue, (void *)&event, portTICK_PERIOD_MS) == pdTRUE)
            {
                memset(buffer, 0, sizeof(buffer));
                switch (event.type)
                {
                    case UART_DATA:
                        uart_read_bytes(UART_NUM, data, event.size, portMAX_DELAY);
                        PRINTFC_SERIAL("Received %d bytes", event.size);
                        PRINTFC_SERIAL("Data: %s", data);

                        // Send the data to the queue
                        xQueueSend(serial_msg_queue, (void *)data, portMAX_DELAY);

                        uart_write_bytes(UART_NUM, (const char *)data, event.size);
                        break;
                    default:
                        break;
                }
            }
            xSemaphoreGive(xSemaphore_serial);
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    vTaskDelete(NULL);
}