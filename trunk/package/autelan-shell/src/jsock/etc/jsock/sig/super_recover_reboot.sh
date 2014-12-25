#!/bin/bash

. ${__ROOTFS__}/etc/jsock/jsock.in

#
#call by jsock.cb
#
#no args
#
main() {
	jsock_md_send_check || {
		return ${e_bad_board}
	}

	${__ROOTFS__}/etc/jsock/jsig.sh asyn "super_recover_reboot"
}

main "$@"
