package main

import (
	"bytes"
	"crypto/tls"
	"fmt"
	"io"
	"net/http"
)

func main() {
	// Skip certificate verification
	http.DefaultTransport.(*http.Transport).TLSClientConfig = &tls.Config{InsecureSkipVerify: true}

	// Test /spelare/register
	resp, err := http.Post("https://localhost:9191/spelare/register", "application/json", nil)
	if err != nil {
		panic(fmt.Sprintf("Failed to register player: %v", err))
	}
	defer resp.Body.Close()
	body, _ := io.ReadAll(resp.Body)
	fmt.Println("Register Player Response:", string(body))

	// Simulate sending a CSR (you'd generate one manually)
	csrData := []byte("YOUR_CSR_CONTENT_HERE") // Replace with real CSR bytes
	resp, err = http.Post("https://localhost:9191/spelare/csr", "application/x-pem-file", bytes.NewReader(csrData))
	if err != nil {
		panic(fmt.Sprintf("Failed to send CSR: %v", err))
	}
	defer resp.Body.Close()
	body, _ = io.ReadAll(resp.Body)
	fmt.Println("CSR Response:", string(body))

	// Test /start
	startPayload := `{"val": "nu kör vi"}`
	resp, err = http.Post("https://localhost:9191/start", "application/json", bytes.NewBuffer([]byte(startPayload)))
	if err != nil {
		panic(fmt.Sprintf("Failed to start game: %v", err))
	}
	defer resp.Body.Close()
	body, _ = io.ReadAll(resp.Body)
	fmt.Println("Start Game Response:", string(body))
}
