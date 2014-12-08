#!/bin/sh

cdma_on(){
	GPIO=$1
	echo ${GPIO}  > /sys/class/gpio/export
	echo 1   > /sys/class/gpio/gpio${GPIO}/active_low
	echo out > /sys/class/gpio/gpio${GPIO}/direction
	echo 0   > /sys/class/gpio/gpio${GPIO}/value
}

cdma_on 13
cdma_on 14

