#!/bin/sh

. /sbin/autelan_functions.sh

main() {
	get_time
	DEBUG=on /etc/jsock/jmsg.sh asyn acc_off {\"date\":\"$Time\"}
}

#main $@
exit 0

