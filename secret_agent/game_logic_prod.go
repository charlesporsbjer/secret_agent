//go:build !debug
// +build !debug

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

	totalPlayers := len(ids)
	ghgCount, sssCount := 0, 0
	switch totalPlayers {
	case 5:
		ghgCount, sssCount = 3, 2
	case 6:
		ghgCount, sssCount = 4, 2
	case 7:
		ghgCount, sssCount = 4, 3
	case 8:
		ghgCount, sssCount = 5, 3
	case 9:
		ghgCount, sssCount = 6, 3
	case 10:
		ghgCount, sssCount = 6, 4
	case 11:
		ghgCount, sssCount = 7, 4
	default:
		ghgCount = totalPlayers / 2
		sssCount = totalPlayers - ghgCount
	}

	for i, pid := range ids {
		p := gameState.Players[pid]
		if i < ghgCount {
			p.Team = GHG
			topic := fmt.Sprintf("/spelare/%s/downlink", p.ID)
			mqttClient.Publish(topic, 2, false, "Du är med i GHG laget")
		} else if i < ghgCount+sssCount {
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
