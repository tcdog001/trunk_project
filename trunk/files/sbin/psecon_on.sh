#!/bin/sh

main() {
	echo 1 > /sys/class/leds/db120:green:psecon/brightness
}

main "$@"
