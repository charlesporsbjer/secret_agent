#ifndef GENERATE_CSR_H
#define GENERATE_CSR_H  

#include "mbedtls/x509_csr.h"
#include "mbedtls/pem.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"

int generate_csr(char *csr_buf, size_t csr_buf_size, const char *player_id);

#endif