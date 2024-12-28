package server

import (
    "github.com/gin-gonic/gin"
)

func SetupRouter() *gin.Engine {
    router := gin.Default()

    // Define HTTPS endpoint
    router.POST("/https", httpsHandler)

    // Define MQTT endpoint
    router.POST("/mqtt", mqttHandler)

    return router
}