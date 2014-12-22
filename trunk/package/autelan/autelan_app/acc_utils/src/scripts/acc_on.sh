#!/bin/bash

cdma_on(){
	GPIO=$1
	echo ${GPIO}  > /sys/class/gpio/export
	echo 1   > /sys/class/gpio/gpio${GPIO}/active_low
	echo out > /sys/class/gpio/gpio${GPIO}/direction
	echo 0   > /sys/class/gpio/gpio${GPIO}/value
}

main() {
	/etc/init.d/led reload

	cdma_on 13
	cdma_on 14
}

#main $@
exit 0
