#!/bin/bash

. /sbin/autelan_functions.in

main() {
	restart_prepare

	RESL=$(/bin/cat /sys/kernel/debug/gpio  | /usr/bin/awk -F ' ' '/gpio-17/{print $5}')
	if [ "$RESL" == "hi" ]; then
		echo "$0: off-delay"
		sync
		echo 1 > /sys/devices/platform/leds-gpio/leds/db120:green:offdelay/brightness
	else
		echo "$0: reboot"
		sync
		reboot
	fi
}

main "$@"

