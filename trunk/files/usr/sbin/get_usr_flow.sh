#!/bin/bash
USR_TC_LOG_PATH="/tmp/usr"
ONLINE_UER_LIST="${USR_TC_LOG_PATH}/usr_online.log"
CURRENT_USR_LIST="${USR_TC_LOG_PATH}/current_usr.log"
CURRENT_MAC_LIST="${USR_TC_LOG_PATH}/current_mac.log"
OLD_USR_LIST="${USR_TC_LOG_PATH}/old_usr.log"
ARP_LIST="${USR_TC_LOG_PATH}/usr_arp.log"
FLOW_WIFIUP="${USR_TC_LOG_PATH}/traffic_control_wifiup.log"
FLOW_WIFIDOWN_2="${USR_TC_LOG_PATH}/traffic_control_wifidown_2.log"
FLOW_WIFIDOWN_5="${USR_TC_LOG_PATH}/traffic_control_wifidown_5.log"
FLOW_3GUP="${USR_TC_LOG_PATH}/traffic_control_3Gup.log"
FLOW_3GDOWN="${USR_TC_LOG_PATH}/traffic_control_3Gdown.log"
UV_TIME_FLOW="${USR_TC_LOG_PATH}/uv_time_flow.log"

merge_files() {
	local target_file=$1
	local target_function=$2; shift 2
	local parameter="$@"
	
	local -a array=($(cat ${target_file}))
	local i
	local count=${#array[*]}
	
	for ((i=0; i<count; i++)); do
		local key=${array[$i]}
		${target_function} "${key}" ${parameter}
	done
}


get_flow() {
	tc -s -d class show dev eth0 > ${USR_TC_LOG_PATH}/tc.log
	tc -s -d class show dev wlan0-1 >> ${USR_TC_LOG_PATH}/tc.log
	tc -s -d class show dev wlan1 >> ${USR_TC_LOG_PATH}/tc.log
	echo "wifi upload traffic control" > ${FLOW_WIFIUP}
	echo "wifi 2.4g downlod traffic control" > ${FLOW_WIFIDOWN_2}
	echo "wifi 5.8g downlod traffic control" > ${FLOW_WIFIDOWN_5}
	echo "3G upload traffic control" > ${FLOW_3GUP}
	echo "3G downlod traffic control" > ${FLOW_3GDOWN}
	local usr_init=192.168.0.
	local -a intf=($(cat ${USR_TC_LOG_PATH}/tc.log | grep class | awk '{print $3}'))
	local -a flow=($(cat ${USR_TC_LOG_PATH}/tc.log | grep Sent | awk '{print $2}'))
	local i
	local count=${#intf[*]}

	for ((i=0; i<count; i++)); do
		local handle=${intf[$i]}
		local ip=${handle:4}

		case ${handle:3:1} in
		2)
			echo "intf:${handle} ip:${usr_init}${ip} ${flow[$i]}" >> ${FLOW_WIFIDOWN_2}
			;;
		3)
			echo "intf:${handle} ip:${usr_init}${ip} ,\"wifiup\":\"${flow[$i]}bytes\"" >> ${FLOW_WIFIUP}
			;;
		4)
			echo "intf:${handle} ip:${usr_init}${ip} ,\"3Gup\":\"${flow[$i]}bytes\"" >> ${FLOW_3GUP}
			;;
		5)
			echo "intf:${handle} ip:${usr_init}${ip} ,\"3Gdown\":\"${flow[$i]}bytes\"" >> ${FLOW_3GDOWN}
			;;
		6)
			echo "intf:${handle} ip:${usr_init}${ip} ${flow[$i]}" >> ${FLOW_WIFIDOWN_5}
			;;
		*)
			echo ERROR
			;;
		esac
	done	
}

get_interface_flow() {
	local usr_ip=$1
	local file_flow=$2
	
	cat ${file_flow} | grep "${usr_ip} " | awk '{print $3}'
	
}

get_usr_flow() {
	local usr_ip=$1
	local usr_flow_wifiup=$(get_interface_flow "${usr_ip}" "${FLOW_WIFIUP}")
	local usr_flow_wifidown_2=$(get_interface_flow "${usr_ip}" "${FLOW_WIFIDOWN_2}")
	local usr_flow_wifidown_5=$(get_interface_flow "${usr_ip}" "${FLOW_WIFIDOWN_5}")
	local usr_flow_wifidown
	((usr_flow_wifidown=${usr_flow_wifidown_2}+${usr_flow_wifidown_5}))
	local usr_flow_3Gup=$(get_interface_flow "${usr_ip}" "${FLOW_3GUP}")
	local usr_flow_3Gdown=$(get_interface_flow "${usr_ip}" "${FLOW_3GDOWN}")
	echo "${usr_flow_wifiup},\"wifidown\":\"${usr_flow_wifidown}bytes\"${usr_flow_3Gup}${usr_flow_3Gdown}" > ${USR_TC_LOG_PATH}/usr_flow.log
}

write_uv_time_flow() {
	local target_file=$1
	local key=$2
	local time_current=$3
	
	local usr_time=$(cat ${target_file} | grep ${key} | awk -F '|' '{print $2}')
	local usr_ip=$(cat ${target_file} | grep ${key} | awk -F '|' '{print $1}')
	get_usr_flow ${usr_ip}
	local ip_flow=$(cat ${USR_TC_LOG_PATH}/usr_flow.log)
	echo "{${usr_time},\"endtime\":\"${time_current}\"${ip_flow}}" >> ${UV_TIME_FLOW} 
}

get_usr_starttime() {

	local mac_current=$1
	local usr_online_time=$2
	
	local usr_ip_line=$(cat ${ARP_LIST} | grep ${mac_current})
	
	if [ "${usr_ip_line}" = "" ]; then
		sed -i "/${mac_current}/d" ${CURRENT_MAC_LIST} 
	else
		cat ${ONLINE_UER_LIST} | grep "${mac_current}" ; local usr_state=$?
		if [ ${usr_state} -ne 0 ]; then 
			local usr_ip=$(echo ${usr_ip_line} | awk -F '(' '{print $2}' | awk -F ')' '{print $1}')
			echo "${usr_ip}|\"mac\":\"${mac_current}\",\"IP\":\"${usr_ip}\",\"starttime\":\"${usr_online_time}\"" >> ${ONLINE_UER_LIST} 
		fi
	fi
}

get_usr_endtime() {
	
	local mac_old=$1
	local usr_offline_time=$2
	
	write_uv_time_flow "${ONLINE_UER_LIST} " "${mac_old}" "$usr_offline_time" 
	sed -i "/${mac_old}/d" ${ONLINE_UER_LIST} 


}


get_usr_time() {
	local usr_mac=$1
	local file_mac_2=$2
	local get_time_function=$3
	
	cat ${file_mac_2} | grep ${usr_mac}; local usr_state=$?
	if [ ${usr_state} -ne 0 ]; then 
		local time_current=$(date '+%F-%H-%M-%S')
		${get_time_function} "${usr_mac}" "${time_current}" 
		
	fi
}
get_uv_time_flow() {
	local usr_time_info=$1
	
	local usr_ip=$(echo ${usr_time_info} | awk -F '|' '{print $1}')
	write_uv_time_flow "${ONLINE_UER_LIST} " "${usr_ip}|" "" 
}


main() {
	if [ ! -d "${USR_TC_LOG_PATH}" ]
	then
	mkdir -p ${USR_TC_LOG_PATH}
	fi
	
	touch ${OLD_USR_LIST}
	touch ${ARP_LIST}
	touch ${ONLINE_UER_LIST}

	while :
	do
	cat /dev/null > ${UV_TIME_FLOW}
	/usr/bin/tftp -g -l /tmp/usr_arp.log -r /tmp/tftp/usr_arp.log 1.0.0.2 2>/dev/null
	sleep 1
	cat /tmp/usr_arp.log | grep "192.168.0." > ${ARP_LIST}
	
	iw dev wlan0-1 station dump | grep Station | awk '{print $2}' > ${CURRENT_USR_LIST}
	iw dev wlan1 station dump | grep Station | awk '{print $2}' >> ${CURRENT_USR_LIST}
	cp ${CURRENT_USR_LIST} ${CURRENT_MAC_LIST}
	get_flow

	merge_files "${CURRENT_USR_LIST}" "get_usr_time" "${OLD_USR_LIST}" "get_usr_starttime"
	merge_files "${OLD_USR_LIST}" "get_usr_time" "${CURRENT_USR_LIST}" "get_usr_endtime"
	merge_files "${ONLINE_UER_LIST}" "get_uv_time_flow"
	
	mv ${CURRENT_MAC_LIST} ${OLD_USR_LIST}
	
	/usr/bin/tftp -pl ${UV_TIME_FLOW} -r /opt/log/flow/user/uv_time_flow.log 1.0.0.2 2>/dev/null
	
	sleep 300
	done
	

}

main "$@"
