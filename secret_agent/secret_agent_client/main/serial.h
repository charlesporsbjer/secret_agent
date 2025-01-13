#ifndef SERIAL_H
#define SERIAL_H

<<<<<<< HEAD
#include <stddef.h>

void serial_task(void *pvParameters);

void create_topic(char *topic, size_t topic_size, const char *base_topic, char* player_id);

=======
#include "driver/uart.h"
#include "stdio.h"

void serial_task(void *pvParameters);

char* read_uart_data(uint8_t* data, char* input_buffer, int* buffer_index);
>>>>>>> patriks

#endif