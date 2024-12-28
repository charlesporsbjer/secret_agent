package main

import (
	"crypto/tls"
	"fmt"
	"log"
	"net/http"
)

func main() {
	// Load server certificates
	cert, err := tls.LoadX509KeyPair("./go_cert/go_server.crt", "./go_cert/go_server.key")
	if err != nil {
		log.Fatalf("Failed to load server certificates: %v", err)
	}

	// TLS configuration
	tlsConfig := &tls.Config{
		Certificates: []tls.Certificate{cert},
		ClientAuth:   tls.NoClientCert, // Change to mutual TLS (RequireAndVerifyClientCert)
		MinVersion:   tls.VersionTLS12,
	}

	// Create HTTPS server
	server := &http.Server{
		Addr:      ":9191",
		TLSConfig: tlsConfig,
	}

	// Register endpoints
	http.HandleFunc("/spelare/register", playerRegistrationHandler)
	http.HandleFunc("/spelare/csr", playerCSRHandler)
	http.HandleFunc("/start", startGameHandler)

	fmt.Println("Server is running on https://localhost:9191")
	if err := server.ListenAndServeTLS("./go_cert/go_server.crt", "./go_cert/go_server.key"); err != nil {
		log.Fatalf("Server failed: %v", err)
	}
}

func playerRegistrationHandler(w http.ResponseWriter, r *http.Request) {
	fmt.Println("Player registration request received.")
	if r.Method != http.MethodPost {
		http.Error(w, "Only POST requests are allowed", http.StatusMethodNotAllowed)
		return
	}
	// Example response (generate unique ID later)
	playerID := "1234"
	response := fmt.Sprintf(`{"id": "%s"}`, playerID)
	w.Header().Set("Content-Type", "application/json")
	w.WriteHeader(http.StatusOK)
	w.Write([]byte(response))
}

func playerCSRHandler(w http.ResponseWriter, r *http.Request) {
	fmt.Println("CSR_request received.")
	if r.Method != http.MethodPost {
		http.Error(w, "Only POST requests are allowed", http.StatusMethodNotAllowed)
		return
	}
	// Placeholder logic for CSR handling
	w.WriteHeader(http.StatusOK)
	w.Write([]byte(`{"message": "CSR handled (not yet implemented)"}`))
}

func startGameHandler(w http.ResponseWriter, r *http.Request) {
	fmt.Println("Start game request received.")
	if r.Method != http.MethodPost {
		http.Error(w, "Only POST requests are allowed", http.StatusMethodNotAllowed)
		return
	}
	// Placeholder for starting the game
	w.WriteHeader(http.StatusOK)
	w.Write([]byte(`{"message": "Game started (not yet implemented)"}`))
}
