#!/bin/bash

. /sbin/autelan_functions.sh

main() {
	syslogd

	sleep 20
	save_init_log
	save_last_syslog
}

main "$@"
