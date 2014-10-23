#!/bin/sh

cdma_off(){
	GPIO=$1
	echo ${GPIO}  > /sys/class/gpio/export
	echo 1   > /sys/class/gpio/gpio${GPIO}/active_low
	echo out > /sys/class/gpio/gpio${GPIO}/direction
	echo 1   > /sys/class/gpio/gpio${GPIO}/value
}

cdma_off 13
cdma_off 14

