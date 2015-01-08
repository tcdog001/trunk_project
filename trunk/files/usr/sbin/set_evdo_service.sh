#!/bin/bash

. /sbin/autelan_functions.sh

main() {
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
		/etc/init.d/network reload
	fi
}

main "$@"
