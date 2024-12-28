package main

import (
    "log"
    "net/http"
    "github.com/gin-gonic/gin"
    "crypto/tls"
    "io/ioutil"
)

func main() {
    router := gin.Default()

    // Load server certificate and key
    cert, err := tls.LoadX509KeyPair("certs/server.cert", "certs/server.key")
    if err != nil {
        log.Fatalf("failed to load server certificate and key: %v", err)
    }

    // Create a custom TLS configuration
    tlsConfig := &tls.Config{
        Certificates: []tls.Certificate{cert},
    }

    // Set up HTTPS server
    server := &http.Server{
        Addr:      ":443",
        Handler:   router,
        TLSConfig: tlsConfig,
    }

    // Define routes
    setupRoutes(router)

    // Start HTTPS server
    log.Println("Starting server on https://localhost:443")
    if err := server.ListenAndServeTLS("", ""); err != nil {
        log.Fatalf("failed to start server: %v", err)
    }
}

func setupRoutes(router *gin.Engine) {
    router.GET("/https-endpoint", httpsHandler)
    router.POST("/mqtt-endpoint", mqttHandler)
}

func httpsHandler(c *gin.Context) {
    c.JSON(http.StatusOK, gin.H{"message": "HTTPS request received"})
}

func mqttHandler(c *gin.Context) {
    c.JSON(http.StatusOK, gin.H{"message": "MQTT request received"})
}