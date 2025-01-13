package main

import (
	"crypto/tls"
	"crypto/x509"
	"fmt"
	"log"
	"math/rand"
	"net/http"
	"os"
)

//openssl genpkey -algorithm RSA -out server.key -pkeyopt rsa_keygen_bits:2048
//openssl req -new -key server.key -out server.csr -subj "/C=US/ST=State/L=City/O=Organization/OU=Unit/CN=---------"
// 192.168.0.127

// openssl x509 -req -in server.csr -CA ca.crt -CAkey ca.key -CAcreateserial -out server.crt -days 365 -sha256
func main() {

	serverCert, err := tls.LoadX509KeyPair("./go_cert/server.crt", "./go_cert/server.key")
	if err != nil {
		log.Fatalf("Failed to load server certificate: %v", err)
	}

	certPool := x509.NewCertPool()

	caCert, err := os.ReadFile("./go_cert/ca.crt")
	if err != nil {
		log.Fatalf("Failed to read CA certificate: %v", err)
	}

	certPool.AppendCertsFromPEM(caCert)
	tlsConfig := &tls.Config{
		ClientCAs:    certPool,
		ClientAuth:   tls.VerifyClientCertIfGiven,
		Certificates: []tls.Certificate{serverCert},
	}
	server := &http.Server{
		Addr:      ":9191",
		Handler:   http.DefaultServeMux,
		TLSConfig: tlsConfig,
	}

	http.HandleFunc("/spelare/test", clientTestHandler)

	// // Register endpoints
	//http.HandleFunc("/spelare/register", playerRegistrationHandler)
	http.HandleFunc("/spelare/register", playerRegistrationHandler)

	// http.HandleFunc("/spelare/csr", playerCSRHandler)
	// http.HandleFunc("/start", startGameHandler)

	server.ListenAndServeTLS("", "")

	// Register endpoints
	// http.HandleFunc("/spelare/register", playerRegistrationHandler)
	// http.HandleFunc("/spelare/csr", playerCSRHandler)
	// http.HandleFunc("/start", startGameHandler)

}

func assignPlayerID_DEBUG() string {
	ids := []string{"12345", "52914", "22005"}
	return ids[rand.Intn(len(ids))]
}

func clientTestHandler(w http.ResponseWriter, r *http.Request) {
	fmt.Println("Client test request received.")
	if r.Method != http.MethodPost {
		http.Error(w, "Only POST requests are allowed", http.StatusMethodNotAllowed)
		return
	}
	playerID := assignPlayerID_DEBUG()
	response := fmt.Sprintf(`{"id": "%s"}`, playerID)
	w.Header().Set("Content-Type", "application/json")
	w.WriteHeader(http.StatusOK)
	w.Write([]byte(response))
}

func playerRegistrationHandler(w http.ResponseWriter, r *http.Request) {
	fmt.Println("Player registration request received.")
	if r.Method != http.MethodPost {
		return
	}
	playerID := assignPlayerID_DEBUG()
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
