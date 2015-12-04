#!/bin/bash

. /sbin/autelan_functions.in

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
