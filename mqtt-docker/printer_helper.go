package main

import (
	"fmt"
)

const (
	black         = "\033[0;30m"
	red           = "\033[0;31m"
	green         = "\033[0;32m"
	yellow        = "\033[0;33m"
	blue          = "\033[0;34m"
	magenta       = "\033[0;35m"
	cyan          = "\033[0;36m"
	white         = "\033[0;37m"
	brightMagenta = "\033[1;35m"
	brightRed     = "\033[1;31m"
	reset         = "\033[0m"
)

func Printfc(color, format string, args ...interface{}) {
	fmt.Printf("%s%s%s\n", color, fmt.Sprintf(format, args...), reset)
}

func PrintBroker(format string, args ...interface{}) {
	fmt.Printf("%sBroker:%s ", green, reset)
	fmt.Printf(format, args...)
	fmt.Println()
}

func PrintUplink(format string, args ...interface{}) {
	fmt.Printf("%sUplink:%s ", blue, reset)
	fmt.Printf(format, args...)
	fmt.Println()
}

func PrintGameState(format string, args ...interface{}) {
	fmt.Printf("%sGameState:%s ", yellow, reset)
	fmt.Printf(format, args...)
	fmt.Println()
}

func PrintPlayerAction(format string, args ...interface{}) {
	fmt.Printf("%sPlayerAction:%s ", magenta, reset)
	fmt.Printf(format, args...)
	fmt.Println()
}

func PrintWarning(format string, args ...interface{}) {
	fmt.Printf("%sWarning:%s ", brightRed, reset)
	fmt.Printf(format, args...)
	fmt.Println()
}
