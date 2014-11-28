#!/bin/sh

. /sbin/autelan_functions.sh
FILENAME=/tmp/vcc.log
local_file_path=/root/vcc

save_file() {
	/bin/cp ${FILENAME}  ${local_file_path}/vcc-quality-${VCC_Time} 2> /dev/null
	/bin/cp ${local_file_path}/vcc-quality-${VCC_Time} ${local_file_path}/vcc.log
}

get_vcc_time() {
#	local time=$(/bin/cat ${FILENAME} |/usr/bin/awk -F ',' '{print $1}' | /bin/sed -n '$p')
	#echo $time
	VCC_Time=$(/bin/cat ${FILENAME} |/usr/bin/awk -F '"' '{print $4}' | /bin/sed -n '$p')
#	VCC_Time=$(echo "${time}" |/usr/bin/awk -F '"' '{print $4}')
	echo "$0: ${Time}"
}

main() {
	if [[ -f "${FILENAME}" ]];then
		get_vcc_time
		save_file
	else
		echo "$0: ${FILENAME} not found"
	fi	
}

main $@
exit 0
