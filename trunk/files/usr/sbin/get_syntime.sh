#!/bin/bash

. /sbin/autelan_functions.in

main() {
	local time=$(get_last_syntime)
	echo "last syn time: $time"

	time=$(get_syntime)
	echo "this syn time: $time"

	time=$(get_time)
	echo "system time  : $time"
	
	local net=""
	local service=""
}

main "$@"
