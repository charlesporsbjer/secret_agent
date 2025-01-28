#ifndef SERIAL_H
#define SERIAL_H

#include "driver/uart.h"
#include "stdio.h"
#include "mqtt_handler.h"
#include "client.h"

void serial_task(void *pvParameters);

char* read_uart_data(uint8_t* data, char* input_buffer, int* buffer_index);
void print_hex_data(uint8_t* data, int len);
void serial_parser(char* data, esp_mqtt_client_handle_t client);

#endif