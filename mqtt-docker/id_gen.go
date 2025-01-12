package main

import (
	"fmt"
	"time"

	"math/rand"
)

var adjectives = []string{
	"Swift", "Mighty", "Shadow", "Silent", "Fierce", "Brave", "Noble", "Cunning",
	"Shadowy", "Fearless", "Radiant", "Legendary", "Stealthy", "Thunderous",
	"Grim", "Venomous", "Sassy", "Clumsy", "Derpy", "Sleepy", "Fancy", "Sneaky",
	"Awkward", "Spicy", "Goofy", "Jolly", "Fluffy", "Wobbly", "Snazzy", "Bouncy", "Cheeky", "Witty",
}

var nouns = []string{
	"Warrior", "Hunter", "Dragon", "Phoenix", "Knight", "Wizard", "Ninja",
	"Sorcerer", "Assassin", "Titan", "Warlord", "Archer", "Valkyrie",
	"Gladiator", "Sentinel", "Samurai", "Goblin", "Duck", "Llama", "Robot",
	"Banana", "Potato", "Panda", "Taco", "Burrito", "Unicorn", "Giraffe",
	"Cat", "Doggo", "Sloth", "Penguin", "Waffle", "Monkey", "Cheeseburger",
}

var suffixes = []string{
	"420", "The_Brave", "Xx", "Pro", "69", "Master", "Overlord",
	"Destroyer", "Dancer", "Champion", "The_Noob", "Yeet", "The_Sneaky",
	"The_Awkward", "King", "Queen", "The_Mighty", "Of_Doom", "The_Jolly",
	"Of_Spicy", "From_Space", "The_Legendary", "Of_Wobble", "The_Derp",
	"Of_Chaos", "The_Fluffy", "Of_Shadows", "Of_Cheese",
}

func GenerateName() string {
	rand.Seed(time.Now().UnixNano())
	adj := adjectives[rand.Intn(len(adjectives))]
	noun := nouns[rand.Intn(len(nouns))]
	suffix := suffixes[rand.Intn(len(suffixes))]
	return fmt.Sprintf("%s_%s_%s", adj, noun, suffix)
}
