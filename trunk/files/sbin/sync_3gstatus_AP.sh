#!/bin/sh

status_3g() {
	local inter_evdo=$( /sbin/ifconfig 3g-evdo |/bin/grep addr ) >/dev/null 2>&1
	local STATUS_3g
	local FILE=/tmp/.ppp/status
	local DOWN=down
	local UP=up

	local PS_WFDG=/tmp/.ps_wifidog
	ps |grep wifidog >$PS_WFDG
	grep /usr/bin/wifidog $PS_WFDG >/dev/null 2>&1
	local WFDG_stat=$?
	local WIFIDOG=/etc/init.d/wifidog

	local core_file=/tmp/wifidog.*.core
	ls -la $core_file >/dev/null 2>&1
	local core_result=$?
	if [ ${core_result} -eq 0 ];then
		rm -rf ${core_file}
	fi

	if [ -z $inter_evdo ];then
		echo $DOWN >$FILE
		if [ $WFDG_stat -eq 1 ];then
			$WIFIDOG start >/dev/null 2>&1
		fi
	else
		echo $UP >$FILE
		if [ $WFDG_stat -eq 0 ];then
			$WIFIDOG stop >/dev/null 2>&1
		fi
	fi
	rm -rf $PS_WFDG 2>/dev/null
}

main() {
	while :
	do
		status_3g 2>/dev/null
		sleep 10
	done
}

main "$@"
