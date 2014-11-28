#!/bin/bash

get_device_company() {
	device_company="`partool -part mtd7 -show product.vendor 2>/dev/null`"
	if [ -z "${device_company}" ];then
		device_company="Autelan"
	fi
	echo "${device_company}"
}


get_host_model() {
	host_model="`partool -part mtd7 -show product.model 2>/dev/null`"
	if [ -z "${host_model}" ];then
		host_model="CS-VIC-2000-C"
	fi
	echo "${host_model}"
}

get_host_sn() {
	host_sn="`partool -part mtd7 -show product.sn 2>/dev/null`"
	if [ -z "${host_sn}" ];then
		host_sn="1234567890"
	fi
	echo "${host_sn}"
}

get_mac() {
	host_mac="`partool -part mtd7 -show product.mac 2>/dev/null`"
	if [ -z "${host_mac}" ];then
		host_mac="00:1F:64:00:00:00"
	fi
	echo "${host_mac}"
}

get_cpu_model() {
	cpu_model="`cat /proc/cpuinfo | awk -F ':' '/cpu model/{print $2}' | cut -c 2- 2>/dev/null`"
	if [ -z "${cpu_model}" ];then
		cpu_model="MIPS 74Kc V4.12"
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
#	echo "1234567890"
	get_host_sn
}

# eth1(lan) mac
get_networkcard_mac() {
	networkcard_mac="`ifconfig eth1 | awk -F ' ' '/HWaddr/{print $5}' 2>/dev/null`"
	if [ -z "${networkcard_mac}" ];then
		networkcard_mac="00:1F:64:00:00:00"
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

get_company_of3g() {
	host_company_of3g="`at_ctrl AT+CGMI |sed -n '4p' 2>/dev/null`"
	if [ -z "${host_company_of3g}" ];then
		host_company_of3g="ZTEMT INCORPORATED"
	fi
	echo "${host_company_of3g}"
}

get_meid_of3g() {
	host_meid_of3g="`at_ctrl AT^MEID |sed -n '4p'| awk -F '0x' '{print $2}' 2>/dev/null`"
	if [ -z "${host_meid_of3g}" ];then
		host_meid_of3g="A0000038DCF7F9D"
	fi
	echo "${host_meid_of3g}"
}

get_imsi_of3g() {
	host_imsi_of3g="`at_ctrl AT+CIMI |sed -n '4p' 2>/dev/null`"
	if [ -z "${host_imsi_of3g}" ];then
		host_imsi_of3g="46003023002354"
	fi
	echo "${host_imsi_of3g}"
}

get_model_Of3g() {
	host_model_Of3g="`at_ctrl AT+CGMM | awk '/MC/{print $0}' 2>/dev/null`"
	if [ -z "${host_model_Of3g}" ];then
		host_model_Of3g="MC271X"
	fi
	echo "${host_model_Of3g}"
}

get_sn_Of3g() {
	host_sn_Of3g="`at_ctrl AT+GSN | awk '/0x/{print $0}' 2>/dev/null`"
	if [ -z "${host_sn_Of3g}" ];then
		host_sn_Of3g="0x80A2CB15"
	fi
	echo "${host_sn_Of3g}"
}

get_iccid() {
	host_iccid="`at_ctrl at+zgeticcid | awk -F ' ' '/ZGETICCID/{print $2}' 2>/dev/null`"
	if [ -z "${host_iccid}" ];then
		host_iccid="89860314700104919938"
	fi
	echo "${host_iccid}"
}

get_hard_version() {
	hardVersion_major=$(echo ${host_sn} | awk -F '' '{print $10}')
	hardVersion_minor=$(echo ${host_sn} | awk -F '' '{print $11$12}')
	if [ -z "${hardVersion}" ];then
		hardVersion=0.00
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
	local sn_Of3g=$(get_sn_Of3g)
	local iccid=$(get_iccid)
	local hard_version=$(get_hard_version)
	local firmware_version=$(get_firmware_version)
	local company_of3g=$(get_company_of3g)
	local meid_of3g=$(get_meid_of3g)	
	local Operators=$(get_Operators)	

	printf '{"hostCompany":"%s","hostModel":"%s","hostsn":"%s","mac":"%s","cpuModel":"%s","cpuSN":"%s","memoryModel":"%s","memorySN":"%s","boardSN":"%s","networkCardMac":"%s","lowFreModel":"%s","lowFreSN":"%s","hignFreModel":"%s","hignFreSN":"%s","gpsModel":"%s","gpsSN":"%s","MEID_3g":"%s","3g_Company":"%s","modelOf3g":"%s","snOf3g":"%s","iccid":"%s","Operators":"%s","hardVersion":"%s","firmwareVersion":"%s"}\n' \
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

main() {
	get_host_sysinfo
}

main "$@"
