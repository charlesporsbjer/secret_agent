#ifndef SERIAL_H
#define SERIAL_H

#include "driver/uart.h"
#include "stdio.h"

void serial_task(void *pvParameters);

char* read_uart_data(uint8_t* data, char* input_buffer, int* buffer_index);

#endif