
Next to file with "user:password" run the following to hash passwords

mosquitto_passwd -U passwordfile


## generarte CA key
openssl genpkey -algorithm RSA -out cert/ca.key -pkeyopt rsa_keygen_bits:2048


## generate sign request for CA
openssl req -new -key cert/ca.key -out cert/ca.csr -subj "/C=SE/ST=Stockholm/L=Stockholm/O=Creekside AB/OU=Edu/CN=chawp.se"

## generte certificate for CA
openssl x509 -req -days 365 -in cert/ca.csr -signkey cert/ca.key -out cert/ca.crt


## generarte mqtt key
openssl genpkey -algorithm RSA -out cert/mqtt.key -pkeyopt rsa_keygen_bits:2048

## generate sign request for mqtt
openssl req -new -key cert/mqtt.key -out cert/mqtt.csr -subj "/C=SE/ST=Stockholm/L=Stockholm/O=Creekside AB/OU=Edu/CN=mqtt.chawp.se"

## generte certificate for mqtt
openssl x509 -req -days 365 -in cert/mqtt.csr -CA cert/ca.crt -CAkey cert/ca.key -CAcreateserial -out cert/mqtt.crt


## generarte server key
openssl genpkey -algorithm RSA -out cert/http.key -pkeyopt rsa_keygen_bits:2048

## generate sign request for server
openssl req -new -key cert/http.key -out cert/http.csr -subj "/C=SE/ST=Stockholm/L=Stockholm/O=Creekside AB/OU=Edu/CN=http.chawp.se"

## generte certificate for server
openssl x509 -req -days 365 -in cert/http.csr -CA cert/ca.crt -CAkey cert/ca.key -CAcreateserial -out cert/http.crt


## Start server
docker compose down; docker build -f Dockerfile -t app-server .; docker compose up -d