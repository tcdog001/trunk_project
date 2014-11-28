#!/bin/sh

. /sbin/autelan_functions.sh

main() {
	/sbin/acc_power_off.sh &
	/sbin/upload_vcc.sh &
	/bin/sleep 5

	get_time
	DEBUG=on /etc/jsock/jmsg.sh asyn acc_off {\"date\":\"$Time\"} &
	echo "$0: DEBUG=on /etc/jsock/jmsg.sh asyn acc_off {\"date\":\"$Time\"}"

	ppp_json_string

	/bin/sleep 5
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

main $@
exit 0

