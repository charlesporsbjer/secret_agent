#ifndef CERTS_H
#define CERTS_H

#include "stdint.h"

extern const uint8_t ca_cert_pem_start[] asm("_binary_ca_crt_start");
extern const uint8_t ca_cert_pem_end[] asm("_binary_ca_crt_end");



#endif