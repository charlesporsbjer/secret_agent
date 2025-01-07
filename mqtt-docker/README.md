
Next to file with "user:password" run the following to hash passwords

mosquitto_passwd -U passwordfile


## generarte CA key
openssl genrsa -des3 -out cert/ca.key 2048

## generate sign request for CA
openssl req -new -key cert/ca.key -out cert/ca.csr

## generte certificate for CA
openssl x509 -req -days 365 -in cert/ca.csr -signkey cert/ca.key -out cert/ca.crt


## generarte mqtt key
openssl genrsa -des3 -out cert/mqtt.key 2048

## generate sign request for mqtt
openssl req -new -key cert/mqtt.key -out cert/mqtt.csr -subj "/C=SE/ST=Stockholm/L=Stockholm/O=Creekside AB/OU=Edu/CN=mqtt.creekside.se"

## generte certificate for mqtt
openssl x509 -req -days 365 -in cert/mqtt.csr -CA cert/ca.crt -CAkey cert/ca.key -CAcreateserial -out cert/mqtt.crt


## generarte server key
openssl genrsa -des3 -out cert/server.key 2048

## generate sign request for server
openssl req -new -key cert/server.key -out cert/server.csr -subj "/C=SE/ST=Stockholm/L=Stockholm/O=Creekside AB/OU=Edu/CN=server.creekside.se"

## generte certificate for server
openssl x509 -req -days 365 -in cert/server.csr -CA cert/ca.crt -CAkey cert/ca.key -CAcreateserial -out cert/server.crt
