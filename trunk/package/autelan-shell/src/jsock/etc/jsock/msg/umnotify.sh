#!/bin/bash
	
. ${__ROOTFS__}/etc/jsock/jsock.in

#
# call by busybox udhcpd
#
#$1:event
#$2:mac
#$3:ip
#
main() {
	jsock_md_send_check || {
		return ${e_bad_board}
	}

	local event="$1"
	local mac="$2"
	local ip="$3"

	local json=$(printf '{"event":"%s","mac":"%s","ip":"%s"}' \
		"${event}" \
		"${mac}" \
		"${ip}")

	${__ROOTFS__}/etc/jsock/jmsg.sh syn umnotify "${json}"
}

main "$@"
