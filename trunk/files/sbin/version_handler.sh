#!/bin/sh

main() {

	local PEER=1.0.0.2
	local RBT_TIMER=50
	local DEBUG_LOG_LOCAL=/tmp/debug_upgrade.log
	local file_path=/tmp/tftp/version/sysupgrade.bin

	local version_peer=` DEBUG=on /etc/jsock/jcmd.sh syn cat /etc/.version `	
	local version_local=` cat /etc/.version `
	
	if [ "$version_peer" ] && [ "$version_peer" == "$version_local" ]; then
		echo "$0: peer($version_peer) == local($version_local)" >> $DEBUG_LOG_LOCAL
	else
		echo "$0: peer($version_peer) != local($version_local)" >> $DEBUG_LOG_LOCAL

		killall -9 tftp
		tftp -gr $file_path $PEER -l /tmp/openwrt1.bin >> $DEBUG_LOG_LOCAL
		if [ "$?" -ne "0" ]; then
			echo "$0: tftp -gr $file_path $PEER failed!" >> $DEBUG_LOG_LOCAL
			DEBUG=on /etc/jsock/jmsg.sh asyn logger "tftp -gr $file_path $PEER failed!" &
			exit 1
		fi

		echo "$0: DEBUG=on /etc/jsock/jmsg.sh asyn upgrade_response {\"version\":\"$file_version\",\"reboot_after\":\"$RBT_TIMER\",\"mode\":\"active\"}" >> $DEBUG_LOG_LOCAL
		DEBUG=on /etc/jsock/jmsg.sh asyn upgrade_response {\"version\":\"$file_version\",\"reboot_after\":\"$RBT_TIMER\",\"mode\":\"active\"}

		killall -9 acc_monitor
#		/sbin/mtd -r write /tmp/openwrt1.bin firmware >> $DEBUG_LOG_LOCAL
		/sbin/sysupgrade -n /tmp/openwrt1.bin >> $DEBUG_LOG_LOCAL
	fi
	exit 0

}

sleep 40
main "$@"

