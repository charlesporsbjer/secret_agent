package handlers

import (
    "crypto/tls"
    "net/http"
    "log"
)

// HTTPSHandler handles HTTPS requests
func HTTPSHandler(w http.ResponseWriter, r *http.Request) {
    // Process the HTTPS request
    w.WriteHeader(http.StatusOK)
    w.Write([]byte("HTTPS request received"))
}

// LoadTLSConfig loads the TLS configuration for the server
func LoadTLSConfig() *tls.Config {
    cert, err := tls.LoadX509KeyPair("certs/server.cert", "certs/server.key")
    if err != nil {
        log.Fatalf("failed to load server certificate: %v", err)
    }

    caCert, err := os.ReadFile("certs/ca.cert")
    if err != nil {
        log.Fatalf("failed to read CA certificate: %v", err)
    }

    caCertPool := x509.NewCertPool()
    caCertPool.AppendCertsFromPEM(caCert)

    return &tls.Config{
        Certificates: []tls.Certificate{cert},
        ClientCAs:    caCertPool,
        ClientAuth:   tls.RequireAndVerifyClientCert,
    }
}