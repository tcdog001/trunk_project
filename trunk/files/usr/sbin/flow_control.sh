#!/bin/bash
FLOW_3GDOWN="/root/usr/traffic_control_3Gdown.log"
ONLINE_UER_LIST="/root/usr/usr_online.log"
LIMIT_LIST="/tmp/limit_list.log"
STOP_LIST="/tmp/stop_list.log"
INET="192.168.0."


change_usr_speed() {
	local usr_ip=$1
	local speed=$2
	local dspeed=$3
	local ip=$(echo ${usr_ip} | awk -F '.' '{print $4}')
	
	tc class change dev eth0 parent 10: classid 10:5${ip} htb rate ${speed} ceil ${dspeed}
	echo "${usr_ip} is limited"
}

stop_usr() {
	local usr_ip=$1
	local ip=$(echo ${usr_ip} | awk -F '.' '{print $4}')
	tc class change dev eth0 parent 10: classid 10:5${ip} htb rate 1bps ceil 1bps
	echo "stop ${usr_ip}"
}

usr_flow_control() {
	local usr_ip=$1
	local flow_limit=$2
	local flow_stop=$3
	local limit_download=$4
	local limit_mdownload=$5
	local flow=$(cat ${FLOW_3GDOWN} | grep "${usr_ip} " | awk -F '"' '{print $4}' | awk -F 'bytes' '{print $1}')
	
	if [ -z ${flow_limit} ] || [ -z ${flow_stop} ] || [ -z ${flow} ]; then
		echo "please enter 3 flow"
		return
	fi
	
	if [ ${flow} -gt ${flow_limit} ]; then
		if [ ${flow} -gt ${flow_stop} ]; then
			stop_usr "${usr_ip}"
			echo "${usr_ip}," >> ${STOP_LIST}
		else
			cat ${LIMIT_LIST} | grep "${usr_ip},"; local limit_state=$?
			if [ ${limit_state} -ne 0 ]; then
				change_usr_speed "${usr_ip}" "${limit_download}" "${limit_mdownload}"
				echo "${usr_ip}," >> ${LIMIT_LIST}
			fi
		fi
	fi
}
##容错？？？
main() {
	local flow_limit=$1
	local flow_stop=$2
	local limit_download="160kbit"
	local limit_mdownload="160kbit"
	touch ${LIMIT_LIST}
	touch ${STOP_LIST}
	
	sleep 100
	
	while :
	do
		local -a array=($(cat ${ONLINE_UER_LIST}))
		local i
		local count=${#array[*]}
	
		for ((i=0; i<count; i++)); do
			local usr_ip=$(echo ${array[$i]} | awk -F '|' '{print $1}')
			#cat ${LIMIT_LIST} | grep "${usr_ip},"; local limit_state=$?
			cat ${STOP_LIST} | grep "${usr_ip},"; local stop_state=$?
			#if [ ${limit_state} -ne 0 ] && [ ${stop_state} -ne 0 ]; then
			if [ ${stop_state} -ne 0 ]; then
				usr_flow_control "${usr_ip}" "${flow_limit}" "${flow_stop}" "${limit_download}" "${limit_mdownload}"
			fi
		done
		sleep 300
	done

}

main "$@"