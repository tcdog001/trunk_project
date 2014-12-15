#!/bin/bash

. /sbin/autelan_functions.sh
localstring=network.evdo.service

get_service() {
	local string=${localstring}
	local result=$(uci get ${string} 2> /dev/null)

	echo ${result}
}
set_service() {
	local string=${localstring}
	local service=$1

	echo "$0: set network.interface.service=${service}"
	uci set ${string}=${service}
}
commit_service() {
	local string=${localstring}
	local service=$1

	uci commit ${string}=${service}
}
set_interface_service() {
	local service=$1

	echo "$0: down up interface evdo"
	ubus call network.interface.evdo down
	set_service ${service}
	ubus call network.interface.evdo up
}
main() {
	local net=""
	local service=""

	net=$(get_3g_net)
	if [[ -z "${net}" ]]; then
		echo "$0: cannot get iccid"
		exit
        fi
	service=$(get_service)
	if [[ -z "${service}" ]]; then
		echo "$0: no such entry: ${localstring}"
		exit
	else
		echo "$0: get network.interface.service=${service}"
	fi

	if [[ ${net} == "WCDMA" || ${net} == "GSM/TD-SCDMA" || ${net} == "TD-SCDMA" ]]; then
		if [[ ${service} != "umts" ]]; then
			set_interface_service umts
			commit_service ${service}
		fi
	fi
	if [[ ${net} == "CDMA2000" ]]; then
		if [[ ${service} != "evdo" ]]; then
                        set_interface_service evdo
			commit_service ${service}
		fi
	fi
}

main "$@"
