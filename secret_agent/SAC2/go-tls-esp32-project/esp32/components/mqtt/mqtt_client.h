#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include <stdint.h>

#define MQTT_MAX_TOPIC_LENGTH 128
#define MQTT_MAX_MESSAGE_LENGTH 256

typedef struct {
    char topic[MQTT_MAX_TOPIC_LENGTH];
    char message[MQTT_MAX_MESSAGE_LENGTH];
} mqtt_message_t;

void mqtt_client_init(const char* broker_url, const char* client_id);
void mqtt_client_connect(void);
void mqtt_client_disconnect(void);
void mqtt_client_publish(const mqtt_message_t* msg);
void mqtt_client_subscribe(const char* topic);
void mqtt_client_loop(void);

#endif // MQTT_CLIENT_H