#!/bin/sh

main() {
	echo 0 > /sys/class/leds/db120:green:psecon/brightness
}

main "$@"
