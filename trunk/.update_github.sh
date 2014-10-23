#!/bin/bash

main() {
	local user="$(whoami)"
	local dir=$(pwd)/.rootfs

	git clone https://github.com/ljf10000/rootfs .rootfs

	case "${user}" in
	"root")
		sudo su liuhuijun -c "./.ap.sh ${dir}"
		sudo su hisisdk -c "./.md.sh ${dir}"
		;;
	"hisisdk")
		sudo su liuhuijun -c "./.ap.sh ${dir}"
		./.md.sh ${dir}
		;;
	"liuhuijun")
		./.ap.sh ${dir}
		sudo su hisisdk -c "./.md.sh ${dir}"
		;;
	*)
		;;
	esac

	rm -fr .rootfs
}

main $@

