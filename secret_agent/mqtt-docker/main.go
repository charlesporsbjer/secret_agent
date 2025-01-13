package main

import (
	"fmt"
	"time"
)

func main() {
	//go startBrokerClient()
	startHTTPSServer()

	// Just to keep main() alive for demonstration
	for i := 10; i > 0; i-- {
		fmt.Println(i)
		time.Sleep(1 * time.Second)
	}
}
