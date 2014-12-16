#!/bin/bash

main() {
	echo 0 > /sys/class/leds/db120\:green\:alarm/brightness
}

main "$@"
