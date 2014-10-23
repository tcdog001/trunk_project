#!/bin/sh

main() {

	local PEER=1.0.0.2
	local RBT_TIMER=50
	local DEBUG_LOG_LOCAL=/tmp/debug_upgrade.log

	if [ $# -lt 1 ]; then
		echo "Usage:"
		echo "      $0 {\"version\":\"1.0.2\",\"tftpname\":\"openwrt-sysupgrade.bin\"}"
		exit 1
	fi
	jmsg_upgrade_request="$*"
	echo "$0: $jmsg_upgrade_request" >> $DEBUG_LOG_LOCAL

	file_version=` echo "$jmsg_upgrade_request" | /usr/bin/jq -j '.version|strings' `
	file_path=` echo "$jmsg_upgrade_request" | /usr/bin/jq -j '.tftpname|strings' `
	echo "$0: version: $file_version, path: $file_path" >> $DEBUG_LOG_LOCAL

	killall -9 version_handler.sh
	killall -9 tftp
	tftp -gr $file_path $PEER -l /tmp/openwrt.bin >> $DEBUG_LOG_LOCAL
	if [ "$?" -ne "0" ]; then
		echo "$0: tftp -gr $file_path $PEER failed!" >> $DEBUG_LOG_LOCAL
		DEBUG=on /etc/jsock/jmsg.sh asyn logger "tftp -gr $file_path $PEER failed!" &
		exit 1
	fi

	echo "$0: DEBUG=on /etc/jsock/jmsg.sh asyn upgrade_response {\"version\":\"$file_version\",\"reboot_after\":\"$RBT_TIMER\"}" >> $DEBUG_LOG_LOCAL
	DEBUG=on /etc/jsock/jmsg.sh asyn upgrade_response {\"version\":\"$file_version\",\"reboot_after\":\"$RBT_TIMER\"}

	killall -9 acc_monitor
#	/sbin/mtd -r write /tmp/openwrt.bin firmware >> $DEBUG_LOG_LOCAL
	/sbin/sysupgrade -n /tmp/openwrt.bin >> $DEBUG_LOG_LOCAL

	exit 0

}

main "$@"

