#!/bin/bash

. /sbin/autelan_functions.sh

main() {
	local iccid=""
	
	while :
	do
		[[ ${iccid} ]] && return
		sleep 1
		iccid=$(report_sim_iccid)
	done	
}

main "$@"
