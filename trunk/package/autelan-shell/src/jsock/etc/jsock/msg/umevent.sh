#!/bin/bash

. ${__ROOTFS__}/etc/jsock/msg/umevent.in

#
#$1:mac
#$2:ip
#$3:state
#
set_user() {
	local mac="$1"
	local ip="$2"
	local state="$3"

	local json=$(printf '{"mac":"%s","ip":"%s","state":"%s"}' \
		"${mac}" \
		"${ip}" \
		"${state}")

	${__ROOTFS__}/etc/jsock/jmsg.sh syn umevent "${json}"
}

#
# call by um
#
#$1:event
#$2:ifname
#$3:mac
#$4:ip
#[$5:reason] just for deauth
#
main() {
	jsock_ap_send_check || {
		return ${e_bad_board}
	}

	local event="$1"
	local ifname="$2"
	local mac="$3"
	local ip="$4"
	local reason="%5"

	umevent_logger event=${event} ifname=${ifname} mac=${mac} ip=${ip} reason=${reason}

	case ${event} in
	connect | disconnect)
		#
		# do nothing, now
		#
		;;
	bind)
		#
		# todo: setup tc rule
		#
		;;
	unbind)
		#
		# todo: delete tc rule
		#
		;;
	auth)
		#
		# first on md, so do nothing
		#
		;;
	deauth)
		case ${reason} in
		onlinetime)
			set_user ${mac} ${ip} ${state_user_onlinelimit}
			;;
		flowlimit)
			set_user ${mac} ${ip} ${state_user_flowlimit}
			;;
		admin | initiative)
			return ${e_nosupport}
			;;
		*)
			return ${e_inval}
			;;
		esac
		;;
	*)
		return ${e_inval}
		;;
	esac
}

main "$@"
