#!/bin/bash

main () {
	local err=0
	
	cp /rom/etc/config/network /etc/config/network
	cp /rom/etc/config/firewall /etc/config/firewall
	
	set_oem_service.sh; err=$?
	[[ ${err} != 0 ]] && /etc/init.d/network reload
	/etc/init.d/firewall restart > /dev/null 2>&1
}

main "$@"

