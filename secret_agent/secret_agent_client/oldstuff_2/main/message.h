#ifndef MESSAGE_H
#define MESSAGE_H

#include "game.h"
#include "mbedtls/ssl.h"

#define MSG_Q_SIZE 5

void msg_init();

bool send_msg(move_t msg);

bool rcv_msg(move_t* msg);

int ssl_send_msg(mbedtls_ssl_context *ssl, move_t msg);

int ssl_rcv_msg(mbedtls_ssl_context *ssl, move_t* msg);

#endif

