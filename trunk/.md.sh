#!/bin/bash

. /etc/profile

main() {
	local src=$1
        local dst=${hisitopdir}/autelan/custom

	if [[ "1" != "$#" || ! -d "${src}" ]]; then
                echo "$0 src"

                return 1
        fi

        rm -fr ${dst}/etc/jsock
        rm -fr ${dst}/etc/utils
        rm -fr ${dst}/etc/xinetd.d
        rm -fr ${dst}/etc/upgrade
        cp -fpR ${src}/etc/* ${dst}/etc/
        cp -fpR ${src}/usr/sbin/* ${dst}/usr/sbin/
}

main "$@"

