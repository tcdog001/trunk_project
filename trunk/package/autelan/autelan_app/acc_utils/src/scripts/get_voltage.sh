#!/bin/bash

main() {
	local val=` /bin/cat /sys/devices/platform/i2c-gpio.0/i2c-0/0-0052/iio\:device0/in_voltage_raw 2> /dev/null `
	if [ "$val" -gt "45" ] && [ "$val" -lt "240" ] ;then
		val=$((val*33*13))
		val=$((val/10))
		val=$((val/256))
		echo "$val(v), Pass!"
	else
		echo "Fail!"
	fi
}

main $@
