package main

import (
	"encoding/json"
	"fmt"
	"strings"
	"sync"
	"time"

	mqtt "github.com/eclipse/paho.mqtt.golang"
)

type Team_enum int

const (
	NOTEAM Team_enum = iota
	GHG    Team_enum = iota
	SSS    Team_enum = iota
)

type Vote string

const (
	NONE_VOTE Vote = ""
	OK        Vote = "ok"
	NEKA      Vote = "neka"
)

type Decision string

const (
	NONE_DECISION Decision = ""
	LYCKAS        Decision = "lyckas"
	SABOTERA      Decision = "sabotera"
)

type Player struct {
	ID              string
	Team            Team_enum // "GHG" or "SSS"
	votedLeader     bool
	leaderVote      Vote // "ok" or "neka"
	voted_ok        bool
	decision        Decision // "lyckas" or "sabotera"
	decisionMade    bool
	mutex           sync.Mutex
	LastMessageTime time.Time
	MessageCooldown time.Duration
}

type Phase int

const (
	PHASE_IDLE Phase = iota
	PHASE_LEADER_VOTE
	PHASE_MISSION
	PHASE_EVALUATE
	PHASE_KICK_VOTE
	// ... osv eller liknande
)

type GameState struct {
	Phase          Phase
	Players        map[string]*Player
	LeaderID       string
	Round          int
	SuccessCount   int
	FailureCount   int
	StartTime      time.Time
	PhaseStartTime time.Time
	PhaseTimeout   time.Duration
	Mutex          sync.Mutex
}

var gameState = &GameState{
	Phase:        PHASE_IDLE,
	Players:      make(map[string]*Player),
	LeaderID:     "", // No leader initially
	Round:        0,
	SuccessCount: 0,
	FailureCount: 0,
}

type TorgetMessage struct {
	ID   string `json:"id"`
	Data string `json:"data"`
}

type NyRundaMessage struct {
	Typ  string   `json:"typ"`
	Data []string `json:"data"`
}

type ValAvLedareMessage struct {
	Typ  string `json:"typ"`
	Data string `json:"data"`
}

type AnnounceSabotageVotesMessage struct {
	Typ  string   `json:"typ"`
	Data []string `json:"data"`
}

type MyndighetenMessage struct {
	Typ  string `json:"typ"`
	Data string `json:"data"`
}

type ValAttSparkaMessage struct {
	Typ  string `json:"typ"`
	Data string `json:"data"`
}

type SparkaLedareMessage struct {
	Typ  string `json:"typ"`
	Data string `json:"data"`
}

var mqttClient mqtt.Client

func startBrokerClient() {
	opts := mqtt.NewClientOptions().
		AddBroker("mqtt://192.168.0.155:8883").
		SetClientID("server").SetUsername("server").SetPassword("mosquitto")

	mqttClient = mqtt.NewClient(opts)
	if token := mqttClient.Connect(); token.Wait() && token.Error() != nil {
		panic(token.Error())
	}
	PrintBroker("Connected to MQTT broker")

	initializeMQTT()
}

func initializeMQTT() {

	for id := range gameState.Players {
		uplinkTopic := fmt.Sprintf("/spelare/%s/uplink", id)
		downlinkTopic := fmt.Sprintf("/spelare/%s/downlink", id)

		PrintBroker("Subscribing to '%s'", uplinkTopic)
		mqttClient.Subscribe(uplinkTopic, 1, handleUplink)
		mqttClient.Subscribe(downlinkTopic, 1, nil) // No handler since server writes only
	}
}

func announceGameStart() {
	msg := MyndighetenMessage{
		Typ:  "spelstart",
		Data: "Spelet har nu startat",
	}
	payload, _ := json.Marshal(msg)
	mqttClient.Publish("/myndigheten", 2, false, payload)

}

func announceLeader(leaderID string) {
	msg := ValAvLedareMessage{
		Typ:  "val av ledare",
		Data: leaderID,
	}
	payload, _ := json.Marshal(msg)
	mqttClient.Publish("/myndigheten", 2, false, payload)
	gameState.Phase = PHASE_LEADER_VOTE

	dmsg := fmt.Sprintf("Det är %d spelare", len(gameState.Players))
	for _, p := range gameState.Players {
		topic := fmt.Sprintf("/spelare/%s/downlink", p.ID)
		mqttClient.Publish(topic, 2, false, dmsg)
	}
}

func handleUplink(client mqtt.Client, msg mqtt.Message) {
	PrintUplink("handleUplink() called")

	topic := msg.Topic()

	// hämta ID
	parts := strings.Split(topic, "/")
	if len(parts) != 4 || parts[0] != "" || parts[1] != "spelare" || parts[3] != "uplink" {
		PrintWarning("Invalid topic format: %s", topic)
		return
	}
	playerID := parts[2]

	// Förhindra att servern processar frekventa meddelanden som spelaren skickar på uplink
	gameState.Mutex.Lock()
	p, exists := gameState.Players[playerID]
	if !exists {
		downlinkTopic := fmt.Sprintf("/spelare/%s/downlink", playerID)
		client.Publish(downlinkTopic, 2, false, "You are no longer part of the game")
		gameState.Mutex.Unlock()
		return
	}
	now := time.Now()
	if !p.LastMessageTime.IsZero() && now.Sub(p.LastMessageTime) < p.MessageCooldown {
		PrintWarning("Player %s is sending messages too frequently.", playerID)
		gameState.Mutex.Unlock()
		return
	}
	p.LastMessageTime = now
	gameState.Mutex.Unlock()

	payload := msg.Payload()
	var data map[string]string
	if err := json.Unmarshal(payload, &data); err != nil {
		PrintWarning("Failed to unmarshal uplink JSON: %v", err)
		return
	}

	val := data["val"]
	PrintUplink("Uplink from player %s, val: %s", playerID, val)

	// Lås GameState
	PrintGameState("'Handle Uplink' Attempting to Lock gamestate")
	gameState.Mutex.Lock()
	PrintGameState("'Handle Uplink' Locked gamestate")

	defer gameState.Mutex.Unlock()
	defer PrintGameState("'Handle Uplink' Unlocking gamestate")

	PrintGameState("Games state: %d", gameState.Phase)
	switch gameState.Phase {
	case PHASE_LEADER_VOTE:
		processLeaderVote(playerID, val)
	case PHASE_MISSION:
		processMissionDecision(playerID, val)
	case PHASE_KICK_VOTE:
		processKickVote(playerID, val)
	default:
		// Ingen relevant fas
		PrintGameState("Player %s sent data in phase %v (ignored)", playerID, gameState.Phase)
	}

	progressGame(client)
}

func processLeaderVote(playerID string, val string) {
	PrintPlayerAction("ID: %s casting its vote.\n", playerID)
	p, exists := gameState.Players[playerID]
	if !exists {
		PrintWarning("PlayerID %s does not exist", playerID)
		return
	}

	switch val {
	case "ok":
		PrintPlayerAction("Player %s voted OK", playerID)
		p.leaderVote = OK
		p.voted_ok = true
	case "neka":
		PrintPlayerAction("Player %s voted NEKA", playerID)
		p.leaderVote = NEKA
		p.voted_ok = false
	default:
		PrintWarning("Invalid vote from player %s: %s", playerID, val)
	}
	p.votedLeader = true
}

func processMissionDecision(playerID string, val string) {
	p, exists := gameState.Players[playerID]
	if !exists {
		PrintWarning("PlayerID %s does not exist during mission decision", playerID)
		return
	}

	switch val {
	case "lyckas":
		PrintPlayerAction("Player %s decided LYCKAS", playerID)
		p.decision = LYCKAS
		p.decisionMade = true
	case "sabotera":
		PrintPlayerAction("Player %s decided SABOTERA", playerID)
		p.decision = SABOTERA
		p.decisionMade = true
	default:
		PrintWarning("Invalid mission decision from player %s: %s", playerID, val)
	}
}

func processKickVote(playerID string, val string) {
	p, exists := gameState.Players[playerID]
	if !exists {
		PrintWarning("PlayerID %s does not exist during kick vote", playerID)
		return
	}

	switch val {
	case "ok":
		PrintPlayerAction("Player %s voted OK to kick", playerID)
		p.leaderVote = OK
	case "neka":
		PrintPlayerAction("Player %s voted NEKA to kick", playerID)
		p.leaderVote = NEKA
	default:
		PrintWarning("Invalid kick vote from player %s: %s", playerID, val)
	}
	p.votedLeader = true
}

func progressGame(client mqtt.Client) {
	fmt.Println("progressing game")
	switch gameState.Phase {
	case PHASE_LEADER_VOTE:
		fmt.Println("checking if all players voted")
		if allVoted() {
			evaluateLeaderVote()
		}
	case PHASE_MISSION:
		if allHaveDecision() {
			evaluateMission()
		}
	case PHASE_KICK_VOTE:
		if allVoted() {
			evaluateKickVote(client)
		}
	}
}

func allVoted() bool {
	var voteCount = 0
	var nonVoters []string
	for _, p := range gameState.Players {
		if p.votedLeader {
			voteCount++
		} else {
			nonVoters = append(nonVoters, p.ID)
		}
	}

	PublishDownlinkAll("Votes: %d out of %d", voteCount, len(gameState.Players))

	if len(nonVoters) > 0 {
		nonVotersList := strings.Join(nonVoters, ", ")
		PublishDownlinkAll("The following players did not vote:\n[%s]", nonVotersList)
		fmt.Printf("The following players did not vote:\n[%s]", nonVotersList)
		return false
	}

	fmt.Println("All players voted!")
	PublishDownlinkAll("All players voted!")
	for _, p := range gameState.Players {
		fmt.Printf("PLAYER '%s' voted: %s\n", p.ID, p.leaderVote)
	}
	return true
}

func allHaveDecision() bool {
	for _, p := range gameState.Players {
		if !p.decisionMade {
			return false
		}
	}
	return true
}

func evaluateLeaderVote() {
	fmt.Println("Evaluating votes")
	var okCount, nekaCount int
	for _, p := range gameState.Players {
		if p.leaderVote == OK {
			okCount++
		} else if p.leaderVote == NEKA {
			nekaCount++
		}
		p.votedLeader = false
		p.leaderVote = NONE_VOTE
	}
	PublishDownlinkAll("Ok votes %d | NEKA votes %d", okCount, nekaCount)
	PrintGameState("Ok votes: %d | NEKA votes: %d", okCount, nekaCount)

	if okCount > nekaCount {
		gameState.Phase = PHASE_MISSION
		announceNewRound()
	} else {
		pickNextLeader()
		announceLeader(gameState.LeaderID)
	}
}

func announceNewRound() {
	fmt.Println("Announcing ny runda")

	approvedPlayers := []string{}
	for _, p := range gameState.Players {
		if p.voted_ok {
			fmt.Printf("Player %s voted OK. \nAdding to 'ApprovedPlayers'\n", p.ID)
			approvedPlayers = append(approvedPlayers, p.ID)
		}
	}

	msg := NyRundaMessage{
		Typ:  "ny runda",
		Data: approvedPlayers,
	}

	payload, _ := json.Marshal(msg)
	PublishMyndigheten(string(payload))
	gameState.Phase = PHASE_MISSION

}

func evaluateMission() {
	fmt.Println("Evaluating Mission")
	var sabotageCount int
	var sssCount int
	var saboteurs []string
	for _, p := range gameState.Players {
		if p.decision == SABOTERA {
			sabotageCount++
			saboteurs = append(saboteurs, p.ID)
		}
		p.decisionMade = false
		p.decision = NONE_DECISION
	}

	fmt.Printf("Sabotage count: %d\n", sabotageCount)
	if sabotageCount >= (sssCount / 2) {
		gameState.FailureCount++
		announceMissionResult(false, saboteurs)
		PublishDownlinkAll("Player chose to sabotage the mission")
		PrintGameState("Players chose to sabotage the mission")
	} else {
		gameState.SuccessCount++
		announceMissionResult(true, saboteurs)
		PrintGameState("Players chose NOT to sabotage the mission")
	}

	gameState.Phase = PHASE_KICK_VOTE

	announceKickVote()
}

func announceKickVote() {
	msg := ValAttSparkaMessage{
		Typ:  "val att sparka",
		Data: gameState.LeaderID,
	}
	payload, _ := json.Marshal(msg)
	PublishMyndigheten(string(payload))
	gameState.Phase = PHASE_KICK_VOTE
}
func announceMissionResult(succeeded bool, saboteurs []string) {
	typ := "uppdrag saboterat"
	if succeeded {
		typ = "uppdrag lyckades"
	}
	msg := map[string]interface{}{
		"typ":  typ,
		"data": gameState.LeaderID,
	}
	payload, _ := json.Marshal(msg)
	PublishMyndigheten(string(payload))

	downlinkMsg := AnnounceSabotageVotesMessage{
		Typ:  "saboterare",
		Data: saboteurs,
	}

	payload, _ = json.Marshal(downlinkMsg)
	PublishDownlinkAll(string(payload))
}

func evaluateKickVote(client mqtt.Client) {
	var okCount, nekaCount int
	for _, p := range gameState.Players {
		if p.leaderVote == OK {
			okCount++
		} else if p.leaderVote == NEKA {
			nekaCount++
		}
		p.votedLeader = false
		p.leaderVote = NONE_VOTE
	}

	if okCount > nekaCount {
		announceKickResult(true, gameState.LeaderID)
		fmt.Printf("Player '%s' has been kicked from the game\n", gameState.LeaderID)
		PublishDownlink(gameState.LeaderID, "Du har blivit sparkad från spelet.")
		delete(gameState.Players, gameState.LeaderID)
	} else {
		announceKickResult(false, gameState.LeaderID)
	}

	pickNextLeader()
	announceLeader(gameState.LeaderID)
	gameState.Phase = PHASE_LEADER_VOTE
}

func announceKickResult(kicked bool, leaderID string) {
	if !kicked {
		PublishMyndigheten("sparka avslogs")
		return
	}
	msg := SparkaLedareMessage{
		Typ:  "sparka ledare",
		Data: leaderID,
	}
	payload, _ := json.Marshal(msg)
	PublishMyndigheten("/myndigheten", payload)
}

func pickNextLeader() {
	fmt.Println("PickNextLeader")

	ids := make([]string, 0, len(gameState.Players))
	for id := range gameState.Players {
		ids = append(ids, id)
	}
	if len(ids) == 0 {
		return
	}

	idx := 0
	for i, id := range ids {
		if id == gameState.LeaderID {
			idx = i
			break
		}
	}
	idx = (idx + 1) % len(ids)
	gameState.LeaderID = ids[idx]
}

func (gs *GameState) addPlayer(playerID string) {

	gs.Mutex.Lock()

	if _, exists := gs.Players[playerID]; exists {
		PublishDownlink(playerID, "You have already been registered to play the game.")
		PrintWarning("Player %s already exists in the game.", playerID)
		return
	}

	gs.Players[playerID] = &Player{
		ID:              playerID,
		Team:            NOTEAM,
		votedLeader:     false,
		leaderVote:      NONE_VOTE,
		voted_ok:        false,
		decision:        NONE_DECISION,
		decisionMade:    false,
		LastMessageTime: time.Time{},
		MessageCooldown: time.Second * 1,
	}

	PrintPlayerAction("Player %s added to the game.", playerID)
	PublishDownlinkAll("%s joined the game!", playerID)
	PublishDownlinkAll("There are currently %d players in the game.", len(gs.Players))
	gs.Mutex.Unlock()
}

func PublishDownlinkAll(format string, args ...interface{}) {
	for _, p := range gameState.Players {
		PublishDownlink(p.ID, format, args...)
	}
	fmt.Printf("Sent Downlink Publish to all: \n"+format+"\n", args...)
}

func PublishDownlinkAllSSS(format string, args ...interface{}) {
	for _, p := range gameState.Players {
		if p.Team == SSS {
			PublishDownlink(p.ID, format, args...)
		}
	}
	fmt.Printf("Sent Downlink Publish to SSS: \n"+format+"\n", args...)
}
func PublishDownlinkAllGHG(format string, args ...interface{}) {
	for _, p := range gameState.Players {
		if p.Team == GHG {
			PublishDownlink(p.ID, format, args...)
		}
	}
	fmt.Printf("Sent Downlink Publish to GHG: \n"+format+"\n", args...)
}

func PublishDownlink(playerID string, format string, args ...interface{}) {
	topic := fmt.Sprintf("/spelare/%s/downlink", playerID)
	msg := fmt.Sprintf(format, args...)
	err := mqttClient.Publish(topic, 2, false, msg)
	if err != nil {
		PrintWarning("Failed to send message to %s: %v", topic, err)
	}
}

func PublishMyndigheten(format string, args ...interface{}) {
	mqttClient.Publish("/myndigheten", 2, false, format)
	fmt.Printf("Sent Myndigheten Publish: \n"+format+"\n", args...)
}

func PublishDownlinkExcluding(excludedIDs []string, format string, args ...interface{}) {
	excludeMap := make(map[string]bool)
	for _, id := range excludedIDs {
		excludeMap[id] = true
	}

	excludedList := strings.Join(excludedIDs, ", ")

	for _, p := range gameState.Players {
		if !excludeMap[p.ID] {
			PublishDownlink(p.ID, format, args...)
		}
	}
	fmt.Printf("Excluding players: [%s]\nMessage sent: \n%s\n", excludedList, fmt.Sprintf(format, args...))
}

func GetGHGPlayerIDs() []string {
	var GHGList []string
	for _, p := range gameState.Players {
		if p.Team == GHG {
			GHGList = append(GHGList, p.ID)
		}
	}
	return GHGList
}

func GetSSSPlayerIDs() []string {
	var SSSList []string
	for _, p := range gameState.Players {
		if p.Team == SSS {
			SSSList = append(SSSList, p.ID)
		}
	}
	return SSSList
}
