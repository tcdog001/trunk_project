#!/bin/bash

copy2liujf() {
	local file=$1

	busybox ftpput -u apboard -p apboard 192.168.15.1 ${file} bin/ar71xx/${file}
}

copy2trunk() {
	local file=$1
	
	echo "copy ${file} to trunk..."	
	sshpass -p autelan scp -o StrictHostKeyChecking=no \
		bin/ar71xx/${file} \
		root@192.168.15.111:/data/share/version/lte-fi/apboard/trunk/${file}
	echo "copy ${file} to trunk"
	echo

	copy2liujf ${file}
}

copy2md() {
        local file=$1
	local ap_file=/home/hisisdk/histb/autelan/custom/etc/sysupgrade.bin

	echo "copy ${file} to md..."
        sshpass -p autelan scp -o StrictHostKeyChecking=no \
		bin/ar71xx/${file} \
                root@192.168.15.111:${ap_file}
	sshpass -p autelan ssh -o StrictHostKeyChecking=no \
		root@192.168.15.111 \
		"chmod 644 ${ap_file} && chown hisisdk:hisisdk ${ap_file}"
        echo "copy ${file} to md"
	echo
}

main() {
#	copy2trunk openwrt-ar71xx-generic-db120-kernel.bin
#	copy2trunk openwrt-ar71xx-generic-db120-rootfs-squashfs.bin
#	copy2trunk openwrt-ar71xx-generic-db120-squashfs-sysupgrade.bin
	
#	copy2md openwrt-ar71xx-generic-db120-squashfs-sysupgrade.bin
}

main $@

