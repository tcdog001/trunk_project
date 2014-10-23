#!/bin/bash

main() {
	local src=$1
	local dst=/home/liuhuijun/apboard/package/autelan-shell/src/jsock/etc/

	if [[ "1" != "$#" || ! -d "${src}" ]]; then
		echo "$0 src"

		return 1
	fi

        rm -fr ${dst}/jsock
        rm -fr ${dst}/utils
        rm -fr ${dst}/xinetd.d

        cp -fpR ${src}/etc/jsock ${dst}
        cp -fpR ${src}/etc/utils ${dst}
        cp -fpR ${src}/etc/xinetd.d ${dst}
        rm -f ${dst}/xinetd.d/tftpd_udp
}

main "$@"

