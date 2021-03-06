#!/bin/bash

. /sbin/autelan_functions.in
FILENAME=/tmp/vcc.log
local_file_path=/root/vcc

save_file() {
	local time="$@"
	local cmd1="/bin/cp ${FILENAME} ${local_file_path}/vcc-quality-${time}"
	local cmd2="/bin/cp ${FILENAME} ${local_file_path}/vcc.log"

	echo "$0: ${cmd1}"
	${cmd1} &
	echo "$0: ${cmd2}"
	${cmd2} &
}

get_vcc_time() {
	local vcc_time=$(/bin/cat ${FILENAME} |/usr/bin/awk -F '"' '{print $4}' | /bin/sed -n '$p')
	echo ${vcc_time}
}

main() {
	local time=""

	if [[ -f "${FILENAME}" ]]; then
		time=$(get_vcc_time)
		[ -z ${time} ] && time=$(get_time)
		save_file ${time}
	else
		echo "$0: ${FILENAME} not found"
	fi	
}

main "$@"
