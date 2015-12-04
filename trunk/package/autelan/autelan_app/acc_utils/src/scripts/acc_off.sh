#!/bin/bash

. /sbin/autelan_functions.in

main() {
	local time=$(get_time)
	/etc/jsock/jmsg.sh asyn acc_off {\"date\":\"${time}\"}
}

#main "$@"

