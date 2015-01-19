#!/bin/bash

. /sbin/autelan_functions.sh

get_device_company() {
	device_company=$(partool -part mtd7 -show product.vendor 2>/dev/null)
	if [[ -z "${device_company}" ]];then
		device_company="INVALID DATA"
	fi
	echo "${device_company}"
}


get_host_model() {
	host_model=$(partool -part mtd7 -show product.model 2>/dev/null)
	if [[ -z "${host_model}" ]];then
		host_model="INVALID DATA"
	fi
	echo "${host_model}"
}

get_host_sn() {
	host_sn=$(partool -part mtd7 -show product.sn 2>/dev/null)
	if [[ -z "${host_sn}" ]];then
		host_sn="INVALID DATA"
	fi
	echo "${host_sn}"
}

get_mac() {
	host_mac=$(partool -part mtd7 -show product.mac 2>/dev/null)
	if [[ -z "${host_mac}" ]];then
		host_mac="INVALID DATA"
	fi
	echo "${host_mac}"
}

get_cpu_model() {
	cpu_model=$(cat /proc/cpuinfo | awk -F ':' '/cpu model/{print $2}' | cut -c 2- 2>/dev/null)
	if [[ -z "${cpu_model}" ]];then
		cpu_model="INVALID DATA"
	fi
	echo "${cpu_model}"
}

get_cpu_sn() {
	echo "000000"
}

get_mem_model() {
	echo "abcdefg"
}

get_mem_sn() {
	echo "000000"
}

get_board_sn() {
	get_host_sn
}

# eth1(lan) mac
get_networkcard_mac() {
	networkcard_mac=$(ifconfig eth1 | awk -F ' ' '/HWaddr/{print $5}' 2>/dev/null)
	if [[ -z "${networkcard_mac}" ]];then
		networkcard_mac="INVALID DATA"
	fi
	echo "${networkcard_mac}"
}

get_lowfre_model() {
	echo "AR9344"
}

get_lowfre_sn() {
	echo "000000"
}

get_hignfre_model() {
	echo "AR9582"
}

get_hignfre_sn() {
	echo "000000"
}

get_gps_model() {
	echo "abcdefg"
}

get_gps_sn() {
	echo "000000"
}

get_Operators() {
	echo "CTCC"
}

get_imsi_of3g() {
	host_imsi_of3g=$(at_ctrl AT+CIMI |sed -n '4p' 2>/dev/null)
	if [[ -z "${host_imsi_of3g}" ]];then
		host_imsi_of3g=$(at_ctrl AT+CIMI |sed -n '4p' 2>/dev/null)
		if [[ -z "${host_imsi_of3g}" ]];then
			host_imsi_of3g=$(at_ctrl AT+CIMI |sed -n '4p' 2>/dev/null)
			if [[ -z "${host_imsi_of3g}" ]];then
				host_imsi_of3g="INVALID DATA"
			fi
		fi
	fi
	echo "${host_imsi_of3g}"
}

get_model_Of3g() {
	host_model_Of3g=$(cat /tmp/3g_model 2>/dev/null)
	if [[ -z "${host_model_Of3g}" ]];then
		host_model_Of3g=$(at_ctrl AT+CGMM | awk  -F '' '/MC/{print $0}' 2>/dev/null)
		if [[ -z "${host_model_Of3g}" ]];then
			host_model_Of3g="INVALID DATA"
		fi
	fi
	echo "${host_model_Of3g}"
}

get_sn_Of3g() {
	host_sn_Of3g=$(at_ctrl AT+GSN | awk -F '' '/0x/{print $0}' |sed -n '1p' 2>/dev/null)
	if [[ -z "${host_sn_Of3g}" ]];then
		host_sn_Of3g=$(at_ctrl AT+GSN | awk -F '' '/0x/{print $0}' |sed -n '1p' 2>/dev/null)
		if [[ -z "${host_sn_Of3g}" ]];then
			host_sn_Of3g=$(at_ctrl AT+GSN | awk -F '' '/0x/{print $0}' |sed -n '1p' 2>/dev/null)
			if [[ -z "${host_sn_Of3g}" ]];then
				host_sn_Of3g="INVALID DATA"
			fi
		fi
	fi
	echo "${host_sn_Of3g}"
}

get_hard_version() {
	hardVersion_major=$(echo ${host_sn} | awk -F '' '{print $10}')
	hardVersion_minor=$(echo ${host_sn} | awk -F '' '{print $11$12}')
	if [[ -z "${hardVersion}" ]];then
		hardVersion="0.00"
	fi
	echo "${hardVersion_major}.${hardVersion_minor}"
}

get_firmware_version() {
	cat /etc/.version
}

get_host_sysinfo() {
	local device_company=$(get_device_company)
	local host_model=$(get_host_model)
	local host_sn=$(get_host_sn)
	local mac=$(get_mac)
	local cpu_model=$(get_cpu_model)
	local cpu_sn=$(get_cpu_sn)
	local mem_model=$(get_mem_model)
	local mem_sn=$(get_mem_sn)
	local board_sn=$(get_board_sn)
	local networkcard_mac=$(get_networkcard_mac)
	local lowfre_model=$(get_lowfre_model)
	local lowfre_sn=$(get_lowfre_sn)
	local hignfre_model=$(get_hignfre_model)
	local hignfre_sn=$(get_hignfre_sn)
	local gps_model=$(get_gps_model)
	local gps_sn=$(get_gps_sn)
	local model_Of3g=$(get_model_Of3g)
	local iccid=$(get_iccid)
	local hard_version=$(get_hard_version)
	local firmware_version=$(get_firmware_version)
	local company_of3g=$(get_company_of3g)
	local meid_of3g=$(report_meid_of3g)	
	local sn_Of3g=${meid_of3g}
	local Operators=$(get_Operators)	

	printf '{"hostCompany":"%s","hostModel":"%s","hostsn":"%s","mac":"%s","cpuModel":"%s","cpuSN":"%s","memoryModel":"%s","memorySN":"%s","boardSN":"%s","networkCardMac":"%s","lowFreModel":"%s","lowFreSN":"%s","hignFreModel":"%s","hignFreSN":"%s","gpsModel":"%s","gpsSN":"%s","MEID_3g":"%s","Company_3g":"%s","modelOf3g":"%s","snOf3g":"%s","iccid":"%s","Operators":"%s","hardVersion":"%s","firmwareVersion":"%s"}\n' \
		"${device_company}" 	\
		"${host_model}" 	\
		"${host_sn}"    	\
		"${mac}"		\
		"${cpu_model}"  	\
		"${cpu_sn}"	     	\
		"${mem_model}"  	\
		"${mem_sn}"      	\
		"${board_sn}"   	\
		"${networkcard_mac}"	\
		"${lowfre_model}"	\
		"${lowfre_sn}"		\
		"${hignfre_model}"	\
		"${hignfre_sn}"		\
		"${gps_model}"		\
		"${gps_sn}"		\
		"${meid_of3g}"		\
		"${company_of3g}"	\
		"${model_Of3g}"		\
		"${sn_Of3g}"		\
		"${iccid}"		\
		"${Operators}"		\
		"${hard_version}" 	\
		"${firmware_version}"
}

check_iccid() {
        i=1
        while  [ $i -le 10 ]
        do
                local iccid_result=$(cat ${json_file} |jq -j '.iccid |strings')
                if [[ -z "${iccid_result}" ]];then
                        iccid_result="INVALID DATA"
                fi
                if [[ "${iccid_result}" = "INVALID DATA" ]];then
                        local iccid_new=$(get_iccid)
                        cat ${json_file} |jq -r '.iccid |strings' |sed -i "s/INVALID DATA/${iccid_new}/" ${json_file}
                else
                        break
                fi
                i=$(($i+1))
		sleep 5
        done
}

check_meid() {
        j=1
        while [ $j -le 10 ]
        do
                local meid_result=$(cat ${json_file} |jq -j '.MEID_3g |strings')
                if [[ -z "${iccid_result}" ]];then
                        iccid_result="INVALID DATA"
                fi
                if [[ "${meid_result}" == "INVALID DATA" ]];then
                        local meid_new=$(report_meid_of3g)
                        cat ${json_file} |jq -r '.MEID_3g |strings' |sed -i "s/INVALID DATA/${meid_new}/" ${json_file}
                        cat ${json_file} |jq -r '.snOf3g |strings' |sed -i "s/INVALID DATA/${meid_new}/" ${json_file}
                else
                        break
                fi
                j=$(($j+1))
		sleep 5
        done
}

main() {
	[[ ! -f ${OEM_MD_FLAG} ]] && return

	sleep 30
        local json_file=/tmp/apinfo.json
        get_host_sysinfo >${json_file}

	check_iccid
	check_meid
	cat ${json_file}
}

main "$@"
