package handlers

import (
    "github.com/eclipse/paho.mqtt.golang"
    "net/http"
    "log"
)

var mqttClient mqtt.Client

func InitMQTT(broker string, clientID string) {
    opts := mqtt.NewClientOptions().AddBroker(broker).SetClientID(clientID)
    mqttClient = mqtt.NewClient(opts)

    if token := mqttClient.Connect(); token.Wait() && token.Error() != nil {
        log.Fatalf("Failed to connect to MQTT broker: %v", token.Error())
    }
}

func Subscribe(topic string, callback mqtt.MessageHandler) {
    if token := mqttClient.Subscribe(topic, 0, callback); token.Wait() && token.Error() != nil {
        log.Printf("Failed to subscribe to topic %s: %v", topic, token.Error())
    }
}

func Publish(topic string, payload interface{}) {
    if token := mqttClient.Publish(topic, 0, false, payload); token.Wait() && token.Error() != nil {
        log.Printf("Failed to publish message to topic %s: %v", topic, token.Error())
    }
}

func MQTTHandler(w http.ResponseWriter, r *http.Request) {
    // Example of handling an MQTT request
    topic := r.URL.Query().Get("topic")
    message := r.URL.Query().Get("message")

    if topic != "" && message != "" {
        Publish(topic, message)
        w.WriteHeader(http.StatusOK)
        w.Write([]byte("Message published"))
    } else {
        w.WriteHeader(http.StatusBadRequest)
        w.Write([]byte("Invalid request"))
    }
}