#!/bin/sh       

. /sbin/autelan_functions.sh
offtime_file=/root/onoff/ap-off
ontime_file=/root/onoff/ap-on

get_ontime() {
	sleep 30
	get_time
	echo ${Time} >>${ontime_file} 2>/dev/null
	sleep 5
	get_time
	echo ${Time} >${offtime_file} 2>/dev/null
}

get_offtime() { 
	while :
	do
		get_time
		echo ${Time} >${offtime_file} 2>/dev/null
		sleep 60
	done
}

main() {
	get_ontime
	get_offtime
}

main "$@"

