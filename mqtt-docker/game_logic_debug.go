//go:build debug
// +build debug

package main

import (
	"fmt"
	"math/rand/v2"
)

func initGameLogic() {
	ids := make([]string, 0, len(gameState.Players))
	for id := range gameState.Players {
		ids = append(ids, id)
	}

	rand.Shuffle(len(ids), func(i, j int) {
		ids[i], ids[j] = ids[j], ids[i]
	})

	half := len(ids) / 2
	for i, pid := range ids {
		p := gameState.Players[pid]
		if i < half {
			p.Team = GHG
			topic := fmt.Sprintf("/spelare/%s/downlink", p.ID)
			mqttClient.Publish(topic, 2, false, "Du är med i GHG laget")
		} else {
			p.Team = SSS
			topic := fmt.Sprintf("/spelare/%s/downlink", p.ID)
			mqttClient.Publish(topic, 2, false, "Du är med i SSS laget")
		}
	}

	if len(ids) > 0 {
		gameState.LeaderID = ids[0]
	}
	gameState.Round = 1
	gameState.SuccessCount = 0
	gameState.FailureCount = 0
	announceLeader(gameState.LeaderID)
}
