#!/bin/bash

. /sbin/autelan_functions.in

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

#
# $1: device, $2: username, $3: password
#
set_uci_network_evdo() {
	local device="$1"
	local username="$2"
	local password="$3"

	local ret1 ret2 ret3 ret
	local string_device=network.evdo.device
	local string_username=network.evdo.username
	local string_password=network.evdo.password

	set_option_value ${string_device} "${device}"; ret1=$?
	set_option_value "${string_username}" "${username}"; ret2=$?
	set_option_value "${string_password}" "${password}"; ret3=$?
	
	if [[ ${ret1} = 1 || ${ret2} = 1 || ${ret3} = 1 ]]; then
		ret=1
	else
		ret=0
	fi
	
	return ${ret}
}

check_evdo_service() {
	local net=""
	local service=""
	local device=""
	local model_3g=""
	local operation=0
	local operation1=0
	local string_service=network.evdo.service
	
	net=$(get_3g_net)
	[[ -z "${net}" ]] && $(exit_log $0 "cannot get iccid")
	model_3g=$(get_3g_model)
	[[ -z "${model_3g}" ]] && $(exit_log $0 "cannot get 3g model")

	if [[ ${net} == "WCDMA" || ${net} == "GSM/TD-SCDMA" || ${net} == "TD-SCDMA" ]]; then
		set_option_value ${string_service} umts; operation=$?
	elif [[ ${net} == "CDMA2000" ]]; then
		set_option_value ${string_service} evdo; operation=$?
	fi
	if [[ "${model_3g}" = SIM6320C ]]; then
		set_uci_network_evdo "/dev/ttyUSB2" "ctnet@mycdma.cn" "vnet.mobi"; operation1=$? 
	elif [[ "${model_3g}" = U8300C ]]; then
		set_uci_network_evdo "/dev/ttyUSB1" "ctnet@mycdma.cn" "vnet.mobi"; operation1=$? 
	#elif [[ "${model_3g}" = MC271X ]]; then
	#	set_uci_network_evdo "/dev/ttyUSB0" "hangmei.m2m" "vnet.mobi"; operation1=$? 
	else
		set_uci_network_evdo "/dev/ttyUSB0" "card" "card"; operation1=$? 
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
	echo "oem.lms=${oem_lms}, oem.portal=${oem_portal}" > ${OEM_MD_FLAG}

	if [[ ${oem_lms} || ${oem_portal} ]]; then
		jsonstr=$(add_json_string "lms" "${oem_lms}" "${jsonstr}")
		jsonstr=$(add_json_string "portal" "${oem_portal}" "${jsonstr}")
		/etc/jsock/jmsg.sh asyn oem_service { ${jsonstr} }
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
		sleep 10
		return 0
	fi
	return 1
}

main "$@"
