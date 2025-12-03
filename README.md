# Secret Agent / Secret Hitler – Simplified (Course Project)

This repository contains a course project implementing a simplified, playable version of **Secret Hitler**. The game focuses on the core mechanic where players are randomly assigned to either the **Good** team or the **Bad** team.  
- The Bad team attempts to eliminate the Good players through coordinated voting.  
- The Good team attempts to identify and vote out the Bad players.

The project combines embedded development, secure networking, and backend server logic to create a distributed multiplayer experience.  
**Note:** Parts of the codebase and prints may include Swedish text, as the associated course was held in Sweden.

---

## Architecture Overview

### ESP-32 (Player Devices)
Each player uses an ESP-32 device that maintains its own **player state** such as role and current status.

Responsibilities:
- Store player role and local game state.  
- Communicate with the server using **MQTT** over Wi-Fi.  
- Provide interaction through LEDs, buttons, serial, or other hardware.  
- Present a unique certificate for device validation before joining the game.

All ESP-32 devices are authenticated using **certificate-based security**. Each device is signed by a local certificate authority, and only devices with valid certificates are accepted by the server.

---

## Go Game Server (Dockerized)

The game server is written in Go and runs inside a **Docker container** for consistency and isolated deployment.

Responsibilities:
- Validate ESP-32 device certificates on connection.  
- Manage player registration and random role assignment.  
- Handle global game state, including turn flow and voting logic.  
- Act as the MQTT broker endpoint or communicate with an external MQTT broker (depending on setup).  
- Synchronize relevant updates with all connected ESP-32 devices.

Running the server in Docker ensures reproducible builds and predictable runtime behavior.

---

## MQTT Communication

All communication between the ESP-32 devices and the server occurs through the **MQTT protocol**, chosen for its lightweight design and suitability for embedded systems.

MQTT is used for:
- Device registration  
- Player state updates  
- Voting messages  
- Server-to-device game state broadcasts

The secure connection layer (TLS + certificates) ensures both confidentiality and device authenticity.

---

## How to Run the Project

### Requirements
- One ESP-32 per player  
- Docker (required for the server)  
- A shared Wi-Fi network  
- A configured certificate authority and signed certificates for each device

### Running the Go Server in Docker
Each ESP-32 must be manually provisioned with:
- Its private key  
- A certificate signed by your certificate authority  
- The server's CA certificate

---

## Disclaimer
This project is intended for educational use and is not an official or complete implementation of Secret H
