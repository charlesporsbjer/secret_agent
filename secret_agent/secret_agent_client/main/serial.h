#ifndef SERIAL_H
#define SERIAL_H

#include <stddef.h>

void serial_task(void *pvParameters);

void create_topic(char *topic, size_t topic_size, const char *base_topic, char* player_id);


#endif