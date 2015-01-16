#!/bin/bash

. /sbin/autelan_functions.sh

get_part_mtd7() {
	local ssid=""
	local ssid_str=$1

	ssid=$(partool -part mtd7 -show ${ssid_str})
	[[ "${ssid}" == "${ssid_str} not exist" ]] && ssid=""
	echo ${ssid}
}

check_wireless_ssid() {
	local operation=0
	local tmp=/tmp/.wireless_string.tmp
	local string_service=wireless
	local oem_ssid=""
	local ssid0=""
	local ssid1=""
	local option=""
	local value=""
	local i=0

	oem_ssid=$(get_part_mtd7 oem.ssid)
	if [[ "${oem_ssid}" ]]; then
		ssid0=config.${oem_ssid}
		ssid1=${oem_ssid}	
	else
		ssid0=${DEF_SSID0}
		ssid1=${DEF_SSID1}
	fi
	echo "$0: ssid0=${ssid0}, ssid1=${ssid1}" >> ${DEBUG_LOG_LOCAL}

	i=0
	uci show wireless | sed -n 's/=/ /g;/ssid/p' > ${tmp}
	while read option value; do
		if [[ ${i} = 0 ]]; then
			echo "$0: ${i}, opt=${option}, old=${value}, new=${ssid0}" >> ${DEBUG_LOG_LOCAL}
			if [[ ${ssid0} != ${value} ]]; then
				set_option_value ${option} ${ssid0}; operation=$?
			fi
		else
			echo "$0: ${i}, opt=${option}, old=${value}, new=${ssid1}" >> ${DEBUG_LOG_LOCAL}
			if [[ ${ssid1} != ${value} ]]; then
				set_option_value ${option} ${ssid1}; operation=$?
			fi
		fi
		((i++))
	done < ${tmp}

	if [[ ${operation} -eq 1 ]]; then
		echo "need commit & reload"
		commit_option_value ${string_service}
	fi
	rm ${tmp}
	
	return ${operation}
}

check_evdo_service() {
	local net=""
	local service=""
	local device=""
	local model_3g=""
	local operation=0
	local operation1=0
	local string_service=network.evdo.service
	local string_device=network.evdo.device

	net=$(get_3g_net)
	[[ -z "${net}" ]] && $(exit_log $0 "cannot get iccid")
	model_3g=$(get_3g_model)
	[[ -z "${model_3g}" ]] && $(exit_log $0 "cannot get 3g model")

	if [[ ${net} == "WCDMA" || ${net} == "GSM/TD-SCDMA" || ${net} == "TD-SCDMA" ]]; then
		set_option_value ${string_service} umts; operation=$?
	elif [[ ${net} == "CDMA2000" ]]; then
		set_option_value ${string_service} evdo; operation=$?
	fi
	if [[ ${model_3g} == SIM6320C ]]; then
		set_option_value ${string_device} "/dev/ttyUSB2"; operation1=$?
	else
		set_option_value ${string_device} "/dev/ttyUSB0"; operation1=$?
	fi
	if [[ ${operation} -eq 1 || ${operation1} -eq 1 ]]; then
		commit_option_value ${string_service}
		return 1
	fi
	return 0
}

check_md_service() {
	local jsonstr=""
	local oem_lms=""
	local oem_portal=""

	oem_lms=$(get_part_mtd7 oem.lms)
	oem_portal=$(get_part_mtd7 oem.portal)
	echo "$0: get oem.lms=${oem_lms}, oem.portal=${oem_portal}" >> ${DEBUG_LOG_LOCAL}

	if [[ ${oem_lms} || ${oem_portal} ]]; then
		jsonstr=$(add_json_string "lms" "${oem_lms}" "${jsonstr}")
		jsonstr=$(add_json_string "portal" "${oem_portal}" "${jsonstr}")
		/etc/jsock/jmsg.sh asyn oem_service { ${jsonstr} }
		echo "{ ${jsonstr} }" > ${OEM_MD_FLAG}
		return 1
	fi
	return 0
}

main() {
	local operation1=0
	local operation2=0
	local operation3=0

	check_md_service; operation3=$?
	check_wireless_ssid; operation2=$?
	check_evdo_service; operation1=$?
	
	if [[ ${operation1} -eq 1 || ${operation2} -eq 1 ]]; then
		delete_3g_firewall_flag	
		/etc/init.d/network reload
		return 0
	fi
	return 1
}

main "$@"
