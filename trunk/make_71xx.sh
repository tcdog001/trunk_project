#/bin/sh

#
#$1:hisitopdir
#
main () {
	local file_version=package/base-files/files/etc/.version
	local hisitopdir=$1
	local dir_md=${hisitopdir}/custom/image

	if [ "" == "${hisitopdir}" ]; then
		echo "you must export hisitopdir!!!"
		exit 1
	fi
	cp -f ${hisitopdir}/version ${file_version}
	
	rm -fr build_dir/target-*/autelan-*
	rm -fr build_dir/target-*/thirdpart-*

	#make V=s
	make
	echo "version=$(cat staging_dir/target-mips_34kc_uClibc-0.9.33.2/root-ar71xx/etc/.version)"

	cp db120-describe bin/ar71xx/
	pushd bin/ar71xx/
	md5sum openwrt-ar71xx-generic-db120-squashfs-sysupgrade.bin > sysupgrade.md5
	tar zcvf lte-openwrt.img openwrt-ar71xx-generic-db120-squashfs-sysupgrade.bin db120-describe sysupgrade.md5

	cp -f openwrt-ar71xx-generic-db120-squashfs-sysupgrade.bin ${dir_md}/sysupgrade.bin
	popd
}

main $@
