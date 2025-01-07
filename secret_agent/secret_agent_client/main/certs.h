#ifndef CERTS_H
#define CERTS_H

#include "stdint.h"

extern const uint8_t client_cert_pem_start[] asm("_binary_client_cert_pem_start");
extern const uint8_t client_cert_pem_end[] asm("_binary_client_cert_pem_end");
extern const uint8_t client_key_pem_start[] asm("_binary_client_key_pem_start");
extern const uint8_t client_key_pem_end[] asm("_binary_client_key_pem_end");
extern const uint8_t server_cert_pem_start[] asm("_binary_go_server_pem_start");
extern const uint8_t server_cert_pem_end[] asm("_binary_go_server_pem_end");

extern const uint8_t ca_cert_start[] asm("_binary_ca_crt_start");
extern const uint8_t ca_cert_end[] asm("_binary_ca_crt_end");
extern const uint8_t ca_key_start[] asm("_binary_ca_key_start");
extern const uint8_t ca_key_end[] asm("_binary_ca_key_end");

extern const uint8_t ca_server_copy[] asm("_binary_ca_server_copy_crt_start");


#endif