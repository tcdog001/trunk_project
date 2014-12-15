#!/bin/sh

. /sbin/autelan_functions.sh

main() {
	local time=$(get_time)
	/etc/jsock/jmsg.sh asyn acc_off {\"date\":\"${time}\"}
}

#main "$@"

