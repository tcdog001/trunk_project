#!/bin/bash

. /sbin/autelan_functions.in

main() {
	local file=${SYNTIME}
	syslogd

	while [[ ! -f ${file} ]]
	do
		sleep 9
	done

	save_init_log
	save_last_syslog
}

main "$@"
