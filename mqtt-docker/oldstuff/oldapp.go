package oldmain

import (
	"bufio"
	"crypto/rand"
	"crypto/rsa"
	"crypto/tls"
	"crypto/x509"
	"encoding/json"
	"encoding/pem"
	"fmt"
	"io"
	"log"
	"math/big"
	"net/http"
	"os"
	"path"
	"sync"
	"time"

	mathrand "math/rand"

	mqtt "github.com/eclipse/paho.mqtt.golang"
)

var (
	serverPrivateKey   *rsa.PrivateKey
	playerCertificates = make(map[string]string)
	usedNames          = make(map[string]bool)
	availableNames     []string
	mqttClient         mqtt.Client
	nameMutex          sync.Mutex
)

type GameState struct {
	Players []string
	Started bool
}

var gameState GameState

func init() {
	key, err := rsa.GenerateKey(rand.Reader, 2048)
	if err != nil {
		log.Fatalf("Failed to generate server private key: %v", err)
	}
	serverPrivateKey = key

	if err := loadNamesFromFile("names.txt"); err != nil {
		log.Fatalf("Failed to load names: %v", err)
	}
}

func main() {
	mqttOpts := mqtt.NewClientOptions().AddBroker("mqtt://mosquitto:8883").SetClientID("server").SetPassword("passwd")
	mqttClient = mqtt.NewClient(mqttOpts)

	if token := mqttClient.Connect(); token.Wait() && token.Error() != nil {
		log.Fatalf("Failed to connect to MQTT broker: %v", token.Error())
	} else {
		log.Println("Connected to MQTT broker")
	}

	t := mqttClient.Subscribe("/myndigheten", 0, func(client mqtt.Client, msg mqtt.Message) {
		log.Printf("Received message: %s\n", msg.Payload())
	})
	t.Wait()
	if t.Error() != nil {
		log.Fatalf("Failed to subscribe to /myndigheten: %v", t.Error())
	}

	cert, err := tls.LoadX509KeyPair("/cert/http.crt", "/cert/http.key")
	if err != nil {
		log.Fatalf("Failed to load server certificates: %v", err)
	}

	certPool := x509.NewCertPool()
	caCert, err := os.ReadFile("/cert/ca.crt")
	if err != nil {
		log.Fatalf("Failed to read CA certificate: %v", err)
	}

	certPool.AppendCertsFromPEM(caCert)

	tlsConfig := &tls.Config{
		Certificates: []tls.Certificate{cert},
		ClientAuth:   tls.VerifyClientCertIfGiven,
		ClientCAs:    certPool,
	}

	server := &http.Server{
		Addr:      ":9191",
		Handler:   http.DefaultServeMux,
		TLSConfig: tlsConfig,
	}

	mqttClient.Subscribe("/torget", 0, nil)
	mqttClient.Subscribe("/spelare/+/uplink", 0, nil)
	mqttClient.Subscribe("/myndigheten", 0, nil)

	http.HandleFunc("/spelare", playerRegistrationHandler)
	http.HandleFunc("/spelare/csr", playerCSRHandler)
	http.HandleFunc("/start", startGameHandler)

	fmt.Println("Server is running on https://localhost:9191")
	if err := server.ListenAndServeTLS("", ""); err != nil {
		log.Fatalf("Server failed: %v", err)
	}
}

func loadNamesFromFile(filename string) error {
	file, err := os.Open(filename)
	if err != nil {
		return err
	}
	defer file.Close()

	scanner := bufio.NewScanner(file)
	for scanner.Scan() {
		availableNames = append(availableNames, scanner.Text())
	}
	return scanner.Err()
}

func selectUniqueName() (string, error) {
	nameMutex.Lock()
	defer nameMutex.Unlock()

	var unusedNames []string
	for _, name := range availableNames {
		if !usedNames[name] {
			unusedNames = append(unusedNames, name)
		}
	}

	if len(unusedNames) == 0 {
		return "", fmt.Errorf("no available names")
	}

	mathrand.Seed(time.Now().UnixNano())
	selectedName := unusedNames[mathrand.Intn(len(unusedNames))]

	usedNames[selectedName] = true
	return selectedName, nil
}

func messageHandler(client mqtt.Client, msg mqtt.Message) {
	topic := msg.Topic()
	payload := msg.Payload()

	switch {
	case topic == "/torget":
		handleTorgetMessage(payload)
	case topic == "/myndigheten":
		handleMyndighetenMessage(payload)
	default:
		if match, _ := path.Match("/spelare/*/uplink", topic); match {
			handleUplinkMessage(topic, payload)
		}
	}
}

func handleTorgetMessage(payload []byte) {
	var message map[string]interface{}
	if err := json.Unmarshal(payload, &message); err != nil {
		log.Printf("Failed to unmarshal /torget message: %v", err)
		return
	}
	log.Printf("Received /torget message: %v", message)
}

func handleMyndighetenMessage(payload []byte) {
	var message map[string]interface{}
	if err := json.Unmarshal(payload, &message); err != nil {
		log.Printf("Failed to unmarshal /myndigheten message: %v", err)
		return
	}
	log.Printf("Received /myndigheten message: %v", message)
}

func handleUplinkMessage(topic string, payload []byte) {
	var message map[string]interface{}
	if err := json.Unmarshal(payload, &message); err != nil {
		log.Printf("Failed to unmarshal uplink message: %v", err)
		return
	}
	log.Printf("Received uplink message on topic %s: %v", topic, message)
}

func playerRegistrationHandler(w http.ResponseWriter, r *http.Request) {
	fmt.Println("Player registration request received.")
	if r.Method != http.MethodPost {
		http.Error(w, "Only POST requests are allowed", http.StatusMethodNotAllowed)
		return
	}

	playerID, err := selectUniqueName()
	if err != nil {
		http.Error(w, "No more player IDs available", http.StatusInternalServerError)
		return
	}
	playerCertificates[playerID] = playerID

	response := fmt.Sprintf(`{"id": "%s"}`, playerID)
	w.Header().Set("Content-Type", "application/json")
	w.WriteHeader(http.StatusOK)
	w.Write([]byte(response))
}

func playerCSRHandler(w http.ResponseWriter, r *http.Request) {
	fmt.Println("CSR request received.")
	if r.Method != http.MethodPost {
		http.Error(w, "Only POST requests are allowed", http.StatusMethodNotAllowed)
		return
	}

	csrBytes, err := io.ReadAll(r.Body)
	if err != nil {
		http.Error(w, "Failed to read CSR", http.StatusBadRequest)
		return
	}

	block, _ := pem.Decode(csrBytes)
	if block == nil || block.Type != "CERTIFICATE REQUEST" {
		http.Error(w, "Failed to decode PEM block containing CSR", http.StatusBadRequest)
		log.Println("Failed to decode PEM block containing CSR")
		return
	}

	csr, err := x509.ParseCertificateRequest(block.Bytes)
	if err != nil {
		http.Error(w, "Invalid CSR format: \n"+string(csrBytes), http.StatusBadRequest)
		log.Println(err)
		return
	}

	if err := csr.CheckSignature(); err != nil {
		http.Error(w, "CSR signature verification failed", http.StatusBadRequest)
		return
	}

	playerID := csr.Subject.CommonName
	if _, exists := playerCertificates[playerID]; !exists {
		http.Error(w, "Player not registered", http.StatusUnauthorized)
		return
	}

	certTemplate := &x509.Certificate{
		SerialNumber: big.NewInt(time.Now().UnixNano()),
		Subject:      csr.Subject,
		NotBefore:    time.Now(),
		NotAfter:     time.Now().Add(365 * 24 * time.Hour),
		KeyUsage:     x509.KeyUsageDigitalSignature,
		ExtKeyUsage:  []x509.ExtKeyUsage{x509.ExtKeyUsageClientAuth},
	}

	certBytes, err := x509.CreateCertificate(rand.Reader, certTemplate, certTemplate, csr.PublicKey, serverPrivateKey)
	if err != nil {
		http.Error(w, "Failed to sign certificate", http.StatusInternalServerError)
		return
	}

	certPEM := pem.EncodeToMemory(&pem.Block{Type: "CERTIFICATE", Bytes: certBytes})
	playerCertificates[playerID] = string(certPEM)

	w.Header().Set("Content-Type", "application/x-pem-file")
	w.WriteHeader(http.StatusOK)
	w.Write(certPEM)
}

func startGameHandler(w http.ResponseWriter, r *http.Request) {
	if r.Method != http.MethodPost {
		http.Error(w, "Only POST requests are allowed", http.StatusMethodNotAllowed)
		return
	}

	if gameState.Started {
		http.Error(w, "Game already started", http.StatusBadRequest)
		return
	}

	var requestBody struct {
		Val string `json:"val"`
	}

	err := json.NewDecoder(r.Body).Decode(&requestBody)
	if err != nil || requestBody.Val != "nu kör vi" {
		http.Error(w, "Invalid request payload", http.StatusBadRequest)
		return
	}

	if len(playerCertificates) == 0 {
		http.Error(w, "No players registered", http.StatusBadRequest)
		return
	}

	for playerID := range playerCertificates {
		gameState.Players = append(gameState.Players, playerID)
	}
	gameState.Started = true

	notifyPlayersOfStart()

	w.WriteHeader(http.StatusOK)
	w.Write([]byte("Game started successfully"))
}

func notifyPlayersOfStart() {
	playersData := struct {
		Typ  string   `json:"typ"`
		Data []string `json:"data"`
	}{
		Typ:  "spelare",
		Data: gameState.Players,
	}

	payload, err := json.Marshal(playersData)
	if err != nil {
		log.Printf("Failed to marshal player data: %v", err)
		return
	}

	token := mqttClient.Publish("/myndigheten", 0, false, payload)
	token.Wait()
	if token.Error() != nil {
		log.Printf("Failed to publish to /myndigheten: %v", token.Error())
	}
}
