#include "message.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "mbedtls/ssl.h"
#include "mbedtls/net_sockets.h"
#include "string.h"
#include "printer_helper.h"

QueueHandle_t tls_msg_queue;

void msg_init() 
{
    // Creating freertos queue to hold msg.
    tls_msg_queue = xQueueCreate(MSG_Q_SIZE, sizeof(move_t));
    if (tls_msg_queue == NULL) 
    {
        PRINTFC_MSG("Failed to create TLS msg queue");
        return;
    }
    PRINTFC_MSG("TLS msg queue initialized.");
}

bool send_msg(move_t msg) 
{
    if (xQueueSend(tls_msg_queue, &msg, portMAX_DELAY) != pdPASS) 
    {
        PRINTFC_MSG("Failed to send message");
        return false;
    }
    PRINTFC_MSG("Message sent: %d", msg);
    return true;
}

bool rcv_msg(move_t* msg) 
{
    if (xQueueReceive(tls_msg_queue, msg, portMAX_DELAY) != pdPASS) 
    {
        PRINTFC_MSG("Failed to recieve message from queue");
        return false;
    }
    PRINTFC_MSG("Message received: %d", *msg);
    return true;
}

int ssl_send_msg(mbedtls_ssl_context *ssl, move_t msg) 
{
    int ret = 0;
    char msg_buffer[sizeof(move_t)];

    memcpy(msg_buffer, &msg, sizeof(move_t));

    ret = mbedtls_ssl_write(ssl, (unsigned char*)msg_buffer, sizeof(move_t));
    if (ret <= 0)
    {
        PRINTFC_MSG("Failed to receive message: %d", ret);
        return false;
    }
    PRINTFC_MSG("Message sent over SSL: %d", msg);
    return true;
}

int ssl_rcv_msg(mbedtls_ssl_context *ssl, move_t* msg)
{
    int ret = 0;
    char msg_buffer[sizeof(move_t)];

    ret = mbedtls_ssl_read(ssl, (unsigned char*)msg_buffer, sizeof(move_t));
    if (ret <= 0) 
    {
        PRINTFC_MSG("Failed to receive message over SSL: %d", ret);
        return false;
    }
    memcpy(msg, msg_buffer, sizeof(move_t));
    PRINTFC_MSG("Message received over SSL: %d", *msg);
    return true;
}