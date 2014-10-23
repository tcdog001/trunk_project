#/bin/sh

. /etc/profile

file_version=package/base-files/files/etc/.version
dir_md=${hisitopdir}/autelan/custom/image

rm -fr build_dir/target-*/autelan-*
rm -fr build_dir/target-*/thirdpart-*
#sshpass -p autelan scp root@192.168.15.111:${hisitopdir}/autelan/version ${file_version}
sudo cp -f ${hisitopdir}/autelan/version ${file_version}
sudo chown liuhuijun:liuhuijun ${file_version}

make V=s
echo "version=$(cat staging_dir/target-mips_34kc_uClibc-0.9.33.2/root-ar71xx/etc/.version)"

cp db120-describe bin/ar71xx/
pushd bin/ar71xx/
md5sum openwrt-ar71xx-generic-db120-squashfs-sysupgrade.bin > sysupgrade.md5
tar zcvf lte-openwrt.img openwrt-ar71xx-generic-db120-squashfs-sysupgrade.bin db120-describe sysupgrade.md5

sudo cp -f openwrt-ar71xx-generic-db120-squashfs-sysupgrade.bin ${dir_md}/sysupgrade.bin
sudo chown hisisdk:hisisdk ${dir_md}/sysupgrade.bin
popd


