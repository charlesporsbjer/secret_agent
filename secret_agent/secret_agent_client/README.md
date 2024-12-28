
# OPENSSL how to generate our certificates

## Generate key for server
openssl genpkey -algorithm RSA -out cert/server_key.pem -pkeyopt rsa_keygen_bits:2048

## Generate sign request
openssl req -new -key cert/server_key.pem -out cert/server_csr.pem

## Generate certificate
openssl req -x509 -key cert/server_key.pem -in cert/server_csr.pem -out cert/server_cert.pem -days 365
