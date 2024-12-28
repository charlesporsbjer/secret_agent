# go-tls-esp32-project/README.md

# Go TLS ESP32 Project

This project implements a secure communication setup between a Go server and an ESP32 device using TLS for HTTPS and MQTT protocols. It includes the necessary certificate files and configuration for both the server and the ESP32 application.

## Project Structure

```
go-tls-esp32-project
├── certs
│   ├── ca.cert        # Certificate Authority certificate
│   ├── ca.key         # Private key for the CA
│   ├── server.cert    # Server's public certificate
│   └── server.key     # Private key for the server
├── esp32
│   ├── main
│   │   ├── main.c     # Entry point for the ESP32 application
│   │   └── sdkconfig   # Configuration settings for the ESP32 project
│   ├── components
│   │   └── mqtt
│   │       ├── mqtt_client.c  # MQTT client implementation
│   │       └── mqtt_client.h   # MQTT client declarations
│   └── kconfig.projbuild  # Configuration options for the ESP32 project
├── server
│   ├── main.go        # Entry point for the Go server application
│   ├── router.go      # HTTP routes definition using Gin framework
│   └── handlers
│       ├── https_handler.go  # Handler for HTTPS requests
│       └── mqtt_handler.go    # Handler for MQTT communication
├── go.mod             # Go module configuration file
└── README.md          # Project documentation
```

## Setup Instructions

1. **Install Go**: Ensure you have Go installed on your machine. You can download it from [golang.org](https://golang.org/dl/).

2. **Install ESP-IDF**: Follow the instructions on the [ESP-IDF website](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html) to set up the ESP-IDF development environment.

3. **Generate Certificates**: Use OpenSSL or a similar tool to generate the required certificate files (`ca.cert`, `ca.key`, `server.cert`, `server.key`) and place them in the `certs` directory.

4. **Configure ESP32**: Edit the `kconfig.projbuild` file in the `esp32` directory to set your Wi-Fi SSID and password.

5. **Build and Flash ESP32**: Navigate to the `esp32` directory and use the ESP-IDF tools to build and flash the application to your ESP32 device.

6. **Run the Go Server**: Navigate to the `server` directory and run the Go server using the command:
   ```
   go run main.go
   ```

## Usage

- The ESP32 will connect to the specified Wi-Fi network and establish secure communication with the Go server over HTTPS and MQTT.
- The Go server will handle incoming HTTPS requests and MQTT messages according to the defined routes and handlers.

## License

This project is licensed under the MIT License. See the LICENSE file for more details.