#include "serial.h"
#include "driver/uart.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "printer_helper.h"
#include "string.h"
#include "shared_resources.h"
#include "stdio.h"

#define UART_NUM UART_NUM_0
#define BUF_SIZE 1024
#define TX_PIN 16
#define RX_PIN 17

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

   // PRINTFC_SERIAL("UART driver installed");
    xEventGroupWaitBits(wifi_event_group, MQTT_CLIEN_CONNECTED_BIT, pdFALSE, pdTRUE, portMAX_DELAY);
    

    while (1)
    {
        char* input_string = read_uart_data(data, input_buffer, &buffer_index);
        if (input_string != NULL) {                     
            mqtt_publish(input_string, mqtt_client);
            buffer_index = 0;  // Reset index   
        }
        vTaskDelay(pdMS_TO_TICKS(100));  // Optional delay
                   
    }
    vTaskDelete(NULL);
}



char* read_uart_data(uint8_t* data, char* input_buffer, int* buffer_index) {
    int len = uart_read_bytes(UART_NUM_0, data, 128, pdMS_TO_TICKS(100));  //nått kan va knas här med pdms
    if (len > 0) {
        for (int i = 0; i < len; i++) {
            if (data[i] == '\n' || data[i] == '\r') {
                 if(*buffer_index>0){
                    input_buffer[*buffer_index]='\0';  // Reset index
                    *buffer_index = 0;  // Reset index
                    return input_buffer;    
                 }                              
            } else {
                if (*buffer_index < 127) {
                    input_buffer[*buffer_index] = data[i];
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
}

void print_hex_data(uint8_t* data, int len) {
    for (int i = 0; i < len; i++) {
        printf("%02x ", data[i]);
    }
    printf("\n");
}
