#!/bin/bash

. ${__ROOTFS__}/etc/utils/utils.in

#
#$1:type
#$2:event
#
main() {
	cat $(stimer_show_file "$@")
}

main "$@"
