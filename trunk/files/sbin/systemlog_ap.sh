#!/bin/bash

. ${__ROOTFS__}/sbin/autelan_functions.sh
. ${__ROOTFS__}/etc/jsock/jsock.in
ERROR_NOSUPPORT="Not supported"

get_gps_time() {
    local time=$(cat /tmp/.gps/gps_time)
    [[ ${time} ]] && echo ${time}
}

get_gps_satellite() {
    local satellite=$(cat /tmp/.gps/gps_satellite)
    [[ ${satellite} ]] && echo ${satellite}
}

get_gps_antenna() {
    echo "${ERROR_NOSUPPORT}"
}
get_gps_antenna_bak() {
    local satellite=$(cat /tmp/.gps/gps_satellite)
    if [[ ${satellite} -eq 0 ]]; then
        echo "open"
    else
        echo "short"
    fi
}

#
# $1: device
#
get_wifi_mode() {
    local device="$1"
    local mode
    local mode_tmp=$(iwinfo ${device} info | awk '/Mode:/{print $2}')

    if [[ "${mode_tmp}" = "Master" ]]; then
        mode="AP"
    else
	mode=${mode_tmp}
    fi
    echo ${mode}
}

#
# $1: device
#
get_wifi_signal() {
    local device="$1"
    local signal=$(iwinfo ${device} info | awk  '/Tx-Power/{print $2}')
    echo ${signal}
}

#
# $1: device
#
get_wifi() {
    local device="$1"
    local mode=$(get_wifi_mode ${device})
    local signal=$(get_wifi_signal ${device})
    echo "${mode}/${signal}"
}

get_3g_hdrcsq() {
    local hdrcsq=$(at_ctrl at^hdrcsq |awk -F ':' '/HDRCSQ/{print $2}' 2>/dev/null)
    [[ ${hdrcsq} ]] && echo ${hdrcsq}
}

get_sim_iccid() {
    local iccid_log=/tmp/.ppp/iccid.log
    local iccid_path=/root/ppp/iccid
    local iccid_success=/root/ppp/iccid_success
    local iccid_succ_num=""
    local iccid_fail=/root/ppp/iccid_fail
    local iccid_fail_num=""
    local mode=$(cat /tmp/3g_model)
    local sim_iccid=""

    [[ "${mode}" = "C5300V" ]] && sim_iccid=$(at_ctrl at+iccid | awk '/SCID:/{print $2}')
    [[ "${mode}" = "DM111" ]] && sim_iccid=$(at_ctrl at+iccid | awk '/ICCID:/{print $2}')
    [[ "${mode}" = "MC271X" ]] && sim_iccid=$(at_ctrl at+zgeticcid | awk '/ZGETICCID/{print $2}')

    if [[ ${sim_iccid} ]]; then
        echo ${sim_iccid} > ${iccid_path}
	iccid_succ_num=$(cat ${iccid_success} 2> /dev/null)
	((iccid_succ_num++))
	echo ${iccid_succ_num} > ${iccid_success}
	echo "$(get_gps_time) success: get iccid=${sim_iccid}" >> ${iccid_log}
    else
        sim_iccid=$(cat ${iccid_path})
	iccid_fail_num=$(cat ${iccid_fail} 2> /dev/null)
	((iccid_fail_num++))
	echo ${iccid_fail_num} > ${iccid_fail}
	echo "$(get_gps_time) warning: get iccid=${sim_iccid}" >> ${iccid_log}
    fi
    echo ${sim_iccid}
}

#
# $1: key
# $2: value; shift 2
# $@: json string
#
add_json_string() {
    local key="$1"
    local value="$2"; shift 2
    local str_old="$@"
    local str_new="\"${key}\":\"${value}\""
    local str_entirety

    if [[ ${str_old} ]]; then
        str_entirety="${str_old},${str_new}"
    else
        str_entirety=${str_new}
    fi

    echo ${str_entirety}
}

#
# $@: json string
#
str_systemlog_ap() {
    local jsonstr="$@"

    jsonstr=$(add_json_string "date" "$(get_gps_time)" "${jsonstr}")
    jsonstr=$(add_json_string "gps_satellite" "$(get_gps_satellite)" "${jsonstr}")
    jsonstr=$(add_json_string "gps_antenna" "$(get_gps_antenna)" "${jsonstr}")
    jsonstr=$(add_json_string "wifi24" "$(get_wifi wlan0)" "${jsonstr}")
    jsonstr=$(add_json_string "wifi58" "$(get_wifi wlan1)" "${jsonstr}")
    jsonstr=$(add_json_string "3G_net" "$(get_3g_net)" "${jsonstr}")
    jsonstr=$(add_json_string "3g_strong" "$(get_3g_hdrcsq)" "${jsonstr}")
    jsonstr=$(add_json_string "sim-iccid" "$(get_sim_iccid)" "${jsonstr}")

    [[ ${jsonstr} ]] && echo ${jsonstr}
}

service() {
    local jsonstr=""

    jsock_ap_send_check || {
        return ${e_bad_board}
    }

    jsonstr={$(str_systemlog_ap ${jsonstr})}

    ${__ROOTFS__}/etc/jsock/jmsg.sh asyn systemlog_ap "${jsonstr}"
    jmsg_logger "ap send msg json:${jsonstr}"
}

main() {
    while :
    do
	service
	sleep 293
    done    
}

main "$@"
