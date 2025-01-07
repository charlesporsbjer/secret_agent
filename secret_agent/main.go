package main

import (
	"crypto/tls"
	"fmt"
	"log"
	"net/http"
)

func main() {
	// Load server certificates
	cert, err := tls.LoadX509KeyPair("./go_cert/ca.crt", "./go_cert/ca.key")
	if err != nil {
		log.Fatalf("Failed to load certificate and key pair: %v", err)
	}

	// Print details of the loaded certificate
	fmt.Printf("Certificate loaded")

	// Now you can use this cert in a TLS server or client
	// For example, to start an HTTPS server:
	server := &http.Server{
		Addr:      ":9191",
		TLSConfig: &tls.Config{Certificates: []tls.Certificate{cert}},
	}

	// // Register endpoints
	//http.HandleFunc("/spelare/register", playerRegistrationHandler)
    
	// http.HandleFunc("/spelare/csr", playerCSRHandler)
	// http.HandleFunc("/start", startGameHandler)

	fmt.Println("Server is running on https://localhost:9191")
	if err := server.ListenAndServeTLS("", ""); err != nil {
		log.Fatalf("Server failed: %v", err)
	}


    http.HandleFunc("/spelare/test", clientTestHandler)
	// Register endpoints
	// http.HandleFunc("/spelare/register", playerRegistrationHandler)
	// http.HandleFunc("/spelare/csr", playerCSRHandler)
	// http.HandleFunc("/start", startGameHandler)

}

func clientTestHandler(w http.ResponseWriter, r *http.Request) {
    fmt.Println("Client test request received.")
    if r.Method != http.MethodPost {
        http.Error(w, "Only POST requests are allowed", http.StatusMethodNotAllowed)
        return
    }
    response := `{"message": "Client test successful"}`
    w.Header().Set("Content-Type", "application/json")
    w.WriteHeader(http.StatusOK)
    w.Write([]byte(response))
}

func playerRegistrationHandler(w http.ResponseWriter, r *http.Request) {
	fmt.Println("Player registration request received.")
	if r.Method != http.MethodPost {
		return
	}
	playerID := "1234"
	response := fmt.Sprintf(`{"id": "%s"}`, playerID)
	w.Header().Set("Content-Type", "application/json")
	w.WriteHeader(http.StatusOK)
	w.Write([]byte(response))
}

// func playerCSRHandler(w http.ResponseWriter, r *http.Request) {
// 	fmt.Println("CSR_request received.")
// 	if r.Method != http.MethodPost {
// 		http.Error(w, "Only POST requests are allowed", http.StatusMethodNotAllowed)
// 		return
// 	}
// 	// Placeholder logic for CSR handling
// 	w.WriteHeader(http.StatusOK)
// 	w.Write([]byte(`{"message": "CSR handled (not yet implemented)"}`))
// }

// func startGameHandler(w http.ResponseWriter, r *http.Request) {
// 	fmt.Println("Start game request received.")
// 	if r.Method != http.MethodPost {
// 		http.Error(w, "Only POST requests are allowed", http.StatusMethodNotAllowed)
// 		return
// 	}
// 	// Placeholder for starting the game
// 	w.WriteHeader(http.StatusOK)
// 	w.Write([]byte(`{"message": "Game started (not yet implemented)"}`))
// }

/*


func main() {
    // Load the server's certificate
    certData, err := ioutil.ReadFile("./go_cert/go_server.crt")
    if err != nil {
        log.Fatalf("Failed to read certificate: %v", err)
    }

    // Parse the certificate
    cert, err := x509.ParseCertificate(certData)
    if err != nil {
        log.Fatalf("Failed to parse certificate: %v", err)
    }

    // You can now use the certificate, for example, to configure a server or verify client certificates
    fmt.Printf("Loaded Certificate: %s\n", cert.Subject.CommonName)

    // Set up the TLS configuration with just the certificate (no key)
    tlsConfig := &tls.Config{
        // Only the certificate is included, no private key is required
        Certificates: []tls.Certificate{},  // Empty certificates, since we're not using private key
        ClientCAs:    nil,                  // Optional: Add CA certs if needed
        ClientAuth:   tls.RequireAnyClientCert, // Optional: For mutual TLS if required
    }

    // Set up the server
    server := &http.Server{
        Addr:      ":8443",  // Port to listen on
        Handler:   http.DefaultServeMux,
        TLSConfig: tlsConfig,
    }

    // Define a simple handler for the server
    http.HandleFunc("/", func(w http.ResponseWriter, r *http.Request) {
        fmt.Fprintln(w, "Hello, secure world!")
    })

    // Start the server with TLS (Note: No certificate or key files here)
    log.Println("Server listening on https://localhost:8443")
    err = server.ListenAndServeTLS("", "")  // Since no certificate and key files are provided, it's set in TLSConfig
    if err != nil {
        log.Fatalf("Failed to start server: %v", err)
    }
}

*/
