#!/bin/sh

. /sbin/autelan_functions.sh

main() {
	/sbin/upload_vcc.sh &
	/bin/sleep 5

	get_time
	DEBUG=on /etc/jsock/jmsg.sh asyn acc_off {\"date\":\"$Time\"} &
	echo "$0: DEBUG=on /etc/jsock/jmsg.sh asyn acc_off {\"date\":\"$Time\"}"

	/bin/sleep 5
	echo "$0: shutdown"
	sync
	echo 1 > /sys/devices/platform/leds-gpio/leds/db120:green:offdelay/brightness
	echo 1 > /sys/devices/platform/leds-gpio/leds/db120:green:shutdown/brightness
}

main $@
exit 0

