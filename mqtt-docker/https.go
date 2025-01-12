package main

import (
	"crypto"
	"crypto/tls"
	"crypto/x509"
	"encoding/json"
	"encoding/pem"
	"fmt"
	"io"
	"io/ioutil"
	"log"
	"math/big"
	"net"
	"net/http"
	"time"
)

var id_counter = 0

const HTTPS_PORT = ":9191"

var ipToName = make(map[string]string)

func loadHTTPSTLSConfig(caFile, certFile, keyFile string) (*tls.Config, error) {
	serverCert, err := tls.LoadX509KeyPair(certFile, keyFile)
	if err != nil {
		return nil, fmt.Errorf("failed to load server key pair: %v", err)
	}

	caCert, err := ioutil.ReadFile(caFile)
	if err != nil {
		return nil, err
	}
	caCertPool := x509.NewCertPool()
	if !caCertPool.AppendCertsFromPEM(caCert) {
		return nil, fmt.Errorf("failed to append CA certificate")
	}
	log.Printf("Server certificate subject: \n%s\n", serverCert.Leaf.Subject)
	log.Printf("Server certificate issuer: \n%s\n", serverCert.Leaf.Issuer)

	for i, certDER := range serverCert.Certificate {
		parsedCert, err := x509.ParseCertificate(certDER)
		if err == nil {
			log.Printf("Server chain cert %d subject: \n%s\n", i, parsedCert.Subject)
			log.Printf("Server chain cert %d issuer: \n%s\n", i, parsedCert.Issuer)
		}
	}

	tlsConfig := &tls.Config{
		ClientCAs:    caCertPool,
		Certificates: []tls.Certificate{serverCert},
		ClientAuth:   tls.VerifyClientCertIfGiven,
	}

	return tlsConfig, nil
}

func startHTTPSServer() {
	fmt.Println("Starting HTTPS server")

	fmt.Println("loadHTTPSTLSConfig")
	tlsConfig, err := loadHTTPSTLSConfig("./cert/ca.crt", "./cert/http.crt", "./cert/http.key")
	if err != nil {
		panic(err)
	}

	http.HandleFunc("/spelare", spelare_handler)
	http.HandleFunc("/spelare/csr", spelare_csr_handler)
	http.HandleFunc("/start", start_handler)

	server := &http.Server{
		Addr:      HTTPS_PORT,
		TLSConfig: tlsConfig,
	}

	fmt.Printf("Server is running on port %s\n", HTTPS_PORT)

	err = server.ListenAndServeTLS("", "")
	if err != nil {
		panic(err)
	}
}

func spelare_handler(w http.ResponseWriter, r *http.Request) {
	gameState.Mutex.Lock()
	if gameState.Phase != PHASE_IDLE {
		fmt.Println("Game is ongoing, not permitting clients to ask for ID")
		http.Error(w, "Game is ongoing, cannot assign ID at this time", http.StatusConflict)
		return
	}
	gameState.Mutex.Unlock()

	body, err := io.ReadAll(r.Body)
	if err != nil {
		http.Error(w, "Failed to read request body", http.StatusInternalServerError)
		return
	}
	defer r.Body.Close()

	fmt.Printf("Incoming request body to /spelare: %s\n", string(body))

	ipPort := r.RemoteAddr
	host, _, err := net.SplitHostPort(ipPort)
	fmt.Printf("IP '%s' sent to /spelare", host)
	if err != nil {
		http.Error(w, "Invalid IP address format", http.StatusBadRequest)
		return
	}

	playerID, found := ipToName[host]
	if !found {
		playerID = GenerateName()
		fmt.Printf("Assigning '%s' to '%s'.", playerID, host)
		ipToName[host] = playerID
	}

	response := map[string]string{"id": playerID}

	w.Header().Set("Content-Type", "application/json")
	w.WriteHeader(http.StatusOK)

	// Write the JSON response
	err = json.NewEncoder(w).Encode(response)
	if err != nil {
		http.Error(w, "Failed to encode response", http.StatusInternalServerError)
	}

	uplinkTopic := fmt.Sprintf("/spelare/%s/uplink", playerID)
	fmt.Printf("subscribing to   '%s'\n", uplinkTopic)
	mqttClient.Subscribe(uplinkTopic, 2, handleUplink)
}

func start_handler(w http.ResponseWriter, r *http.Request) {
	body, err := io.ReadAll(r.Body)
	if err != nil {
		http.Error(w, "Failed to read request body", http.StatusInternalServerError)
		return
	}
	defer r.Body.Close()

	ipPort := r.RemoteAddr
	host, _, err := net.SplitHostPort(ipPort)
	if err != nil {
		http.Error(w, "Invalid IP address format", http.StatusBadRequest)
		return
	}
	PlayerID, found := ipToName[host]
	if !found {
		http.Error(w, "Hey what are you trying to do? You haven't registered to play!.", http.StatusBadRequest)
		return
	}

	var req map[string]string
	err = json.Unmarshal(body, &req)
	if err != nil {
		http.Error(w, "Invalid JSON", http.StatusBadRequest)
		return
	}

	if req["val"] == "nu kör vi" {

		fmt.Println("'Start Handler' Attempting to Lock gamestate")
		gameState.Mutex.Lock()
		if len(gameState.Players) < 5 {
			PrintWarning("'%s' attempted to start the game but failed: Too few players: Player count: %d", PlayerID, len(gameState.Players))
			http.Error(w, "Failed to start game, too few players.", http.StatusBadRequest)
			return
		}
		fmt.Println("'Start handler' Lockig gamestate")
		gameState.Phase = PHASE_LEADER_VOTE
		announceGameStart()
		initGameLogic()
		defer gameState.Mutex.Unlock()
		defer fmt.Println("'Start handler' Unlocking Gamestate")

		w.WriteHeader(http.StatusOK)
		w.Write([]byte(`{"status":"Spelet startar"}`))
	} else {
		http.Error(w, "Bad 'val' field", http.StatusBadRequest)
	}
}

func spelare_csr_handler(w http.ResponseWriter, r *http.Request) {
	ipPort := r.RemoteAddr
	host, _, err := net.SplitHostPort(ipPort)
	if err != nil {
		http.Error(w, "Invalid IP address format", http.StatusBadRequest)
		return
	}

	PlayerID, found := ipToName[host]

	if !found {
		http.Error(w, "Did not find a player ID that belongs to you. You might need to ask for a PlayerID first.", http.StatusBadRequest)
		return
	}

	fmt.Println("[spelare_csr_handler] Reading request body")

	body, err := io.ReadAll(r.Body)
	if err != nil {
		fmt.Println("[spelare_csr_handler] Failed to read request body")
		http.Error(w, "Failed to read request body", http.StatusInternalServerError)
		return
	}
	defer r.Body.Close()

	fmt.Printf("[spelare_csr_handler] Incoming CSR (raw PEM):\n%s\n", string(body))

	fmt.Println("[spelare_csr_handler] Decoding PEM block")
	block, _ := pem.Decode(body)
	if block == nil {
		fmt.Println("[spelare_csr_handler] Failed to decode PEM block")
		http.Error(w, "Failed to decode PEM block", http.StatusBadRequest)
		return
	}

	fmt.Println("[spelare_csr_handler] Parsing CSR from PEM block")
	csr, err := x509.ParseCertificateRequest(block.Bytes)
	if err != nil {
		fmt.Println("[spelare_csr_handler] Failed to parse CSR")
		http.Error(w, "Failed to parse CSR", http.StatusBadRequest)
		return
	}

	fmt.Println("[spelare_csr_handler] Validating CSR signature")
	err = csr.CheckSignature()
	if err != nil {
		fmt.Println("[spelare_csr_handler] CSR signature validation failed")
		http.Error(w, "Invalid CSR signature", http.StatusBadRequest)
		return
	}

	fmt.Println("[spelare_csr_handler] Loading CA certificate and key from files")
	caCertPEM, err := ioutil.ReadFile("./cert/ca.crt")
	if err != nil {
		fmt.Println("[spelare_csr_handler] Failed to load CA certificate")
		http.Error(w, "Failed to load CA certificate", http.StatusInternalServerError)
		return
	}

	caKeyPEM, err := ioutil.ReadFile("./cert/ca.key")
	if err != nil {
		fmt.Println("[spelare_csr_handler] Failed to load CA key")
		http.Error(w, "Failed to load CA key", http.StatusInternalServerError)
		return
	}

	fmt.Println("[spelare_csr_handler] Decoding CA certificate PEM block")
	caBlock, _ := pem.Decode(caCertPEM)
	if caBlock == nil {
		fmt.Println("[spelare_csr_handler] Failed to decode CA certificate PEM block")
		http.Error(w, "Failed to decode CA certificate PEM block", http.StatusInternalServerError)
		return
	}

	fmt.Println("[spelare_csr_handler] Parsing CA certificate from PEM block")
	caCert, err := x509.ParseCertificate(caBlock.Bytes)
	if err != nil {
		fmt.Println("[spelare_csr_handler] Failed to parse CA certificate")
		http.Error(w, "Failed to parse CA certificate", http.StatusInternalServerError)
		return
	}

	caKey, err := tls.X509KeyPair(caCertPEM, caKeyPEM)
	if err != nil {
		fmt.Println("[spelare_csr_handler] Failed to parse CA key pair")
		http.Error(w, "Failed to parse CA key pair", http.StatusInternalServerError)
		return
	}

	caPrivateKey, ok := caKey.PrivateKey.(interface {
		Sign(rand io.Reader, digest []byte, opts crypto.SignerOpts) ([]byte, error)
	})
	if !ok {
		fmt.Println("[spelare_csr_handler] Invalid CA private key type")
		http.Error(w, "Invalid CA private key type", http.StatusInternalServerError)
		return
	}

	fmt.Println("[spelare_csr_handler] Creating signed certificate")
	signedCert, err := x509.CreateCertificate(
		nil,
		&x509.Certificate{
			SerialNumber:          big.NewInt(time.Now().UnixNano()),
			Subject:               csr.Subject,
			NotBefore:             time.Now(),
			NotAfter:              time.Now().AddDate(1, 0, 0), // 1 year validity
			KeyUsage:              x509.KeyUsageDigitalSignature | x509.KeyUsageKeyEncipherment,
			ExtKeyUsage:           []x509.ExtKeyUsage{x509.ExtKeyUsageClientAuth},
			BasicConstraintsValid: true,
		},
		caCert,
		csr.PublicKey,
		caPrivateKey,
	)
	if err != nil {
		fmt.Println("[spelare_csr_handler] Failed to sign certificate")
		http.Error(w, "Failed to sign certificate", http.StatusInternalServerError)
		return
	}

	fmt.Println("[spelare_csr_handler] Encoding the signed certificate in PEM format")
	signedCertPEM := pem.EncodeToMemory(&pem.Block{Type: "CERTIFICATE", Bytes: signedCert})

	fmt.Println("[spelare_csr_handler] Sending signed certificate back to client")
	w.Header().Set("Content-Type", "application/x-pem-file")
	w.WriteHeader(http.StatusOK)
	_, err = w.Write(signedCertPEM)
	if err != nil {
		fmt.Println("[spelare_csr_handler] Failed to write response to client")
	}
	gameState.addPlayer(PlayerID)

}
