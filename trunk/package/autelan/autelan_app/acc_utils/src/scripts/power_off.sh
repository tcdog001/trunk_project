#!/bin/sh

. /sbin/autelan_functions.sh

main() {
	restart_prepare

	echo "$0: shutdown"
	sync
	echo 1 > /sys/devices/platform/leds-gpio/leds/db120:green:offdelay/brightness
	echo 1 > /sys/devices/platform/leds-gpio/leds/db120:green:shutdown/brightness
}

main "$@"

