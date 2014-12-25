#!/bin/bash

. ${__ROOTFS__}/etc/utils/utils.in

main() {
	stimer_start keepalive ok
}

main "$@"
