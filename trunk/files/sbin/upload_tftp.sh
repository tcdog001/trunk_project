#!/bin/sh

. /sbin/autelan_functions.sh

tftp_vcc() {                 
	local PEER_TMP_PATH=/tmp/tftp/log/vcc
	local PEER_OPT_PATH=/opt/log/vcc
	local LOCAL_ROOT_PATH=/root/vcc
	local line=$(ls -la /root/vcc |wc -l)
	local time=$(get_time)

	[ -z "${PEER}" ] && PEER=1.0.0.2       

	ls ${LOCAL_ROOT_PATH} |grep vcc-quality >/dev/null 2>&1
	if [[ $? = 0 ]]; then
		echo "$0: /usr/bin/tftp -pl ${LOCAL_ROOT_PATH}/vcc-quality-${time} -r ${PEER_TMP_PATH}/vcc-quality-${time} ${PEER}" >>${TFTP_LOG}
		for file in $(ls ${LOCAL_ROOT_PATH} |grep vcc-quality)
		do
			/usr/bin/tftp -pl ${LOCAL_ROOT_PATH}/${file} -r ${PEER_TMP_PATH}/${file} ${PEER} 2>/dev/null
			if [[ $? = 0 ]];then
				rm -rf ${LOCAL_ROOT_PATH}/${file}
				echo "$0: ${file} TFTP OK" >>${TFTP_LOG}
				/etc/jsock/jcmd.sh syn mv ${PEER_TMP_PATH}/* ${PEER_OPT_PATH}/ &
			else                                                                                          
				echo "$0: ${file} TFTP NOK" >>${TFTP_LOG}
			fi                                                                                            
			i=$(($i-1)) 
		done                                     
	fi                           
} 

tftp_onofftime() {
	local LOCAL_ROOT_PATH=/root/onoff
	local PEER_TMP_PATH=/tmp/tftp/log/onoff
	local PEER_OPT_PATH=/opt/log/onoff
	local line=$(ls -la /root/onoff |wc -l)
	local time=$(get_time)

	[ -z "${PEER}" ] && PEER=1.0.0.2

	ls ${LOCAL_ROOT_PATH} |grep ap-on-off >/dev/null 2>&1
	if [[ $? = 0 ]];then
		echo "$0: /usr/bin/tftp -pl ${LOCAL_ROOT_PATH}/ap-on-off-${time} -r ${PEER_TMP_PATH}/ap-on-off-${time} ${PEER}" >>${TFTP_LOG}
		for file in $(ls ${LOCAL_ROOT_PATH} |grep ap-on-off)
		do
			/usr/bin/tftp -p -l ${LOCAL_ROOT_PATH}/${file} -r ${PEER_TMP_PATH}/${file} ${PEER} 2>/dev/null
			if [[ $? = 0 ]];then
				rm -rf ${LOCAL_ROOT_PATH}/${file}
				echo "$0: ${file} TFTP OK" >>${TFTP_LOG}
				/etc/jsock/jcmd.sh syn mv ${PEER_TMP_PATH}/* ${PEER_OPT_PATH}/ &
			else
				echo "$0: ${file} TFTP NOK" >>${TFTP_LOG} 
			fi
			i=$(($i-1))
		done	
	fi
}

tftp_pppflow() {
	local LOCAL_ROOT_PATH=/root/ppp
	local PEER_TMP_PATH=/tmp/tftp/data/3g
	local PEER_OPT_PATH=/opt/data/3g
	local STRING=3g-flow-
	local file=""

	[ -z "${PEER}" ] && PEER=1.0.0.2

	for file in $(ls ${LOCAL_ROOT_PATH} |grep ${STRING})
	do
		tftp_RtoM ${LOCAL_ROOT_PATH} ${PEER_TMP_PATH} ${PEER_OPT_PATH} ${PEER} ${file} ${file}
	done
}

tftp_initlog() {
	local LOCAL_ROOT_PATH=/root/log/init
	local PEER_TMP_PATH=/tmp/tftp/log/sys/ap/init
	local PEER_OPT_PATH=/opt/log/sys/ap/init
	local STRING=init-
	local file=""

	[ -z "${PEER}" ] && PEER=1.0.0.2

	for file in $(ls ${LOCAL_ROOT_PATH} |grep ${STRING})
	do
		tftp_RtoM ${LOCAL_ROOT_PATH} ${PEER_TMP_PATH} ${PEER_OPT_PATH} ${PEER} ${file} ${file}
	done
}

tftp_ulog() {
	local LOCAL_ROOT_PATH=/root/log/ulog
	local PEER_TMP_PATH=/tmp/tftp/log/sys/ap/ulog
	local PEER_OPT_PATH=/opt/log/sys/ap/ulog
	local STRING=ulog-
	local file=""

	[ -z "${PEER}" ] && PEER=1.0.0.2

	for file in $(ls ${LOCAL_ROOT_PATH} |grep ${STRING})
	do
		tftp_RtoM ${LOCAL_ROOT_PATH} ${PEER_TMP_PATH} ${PEER_OPT_PATH} ${PEER} ${file} ${file}
	done
}

main() {
	local TFTP_LOG=/tmp/.tftp.log

	sleep 15
	tftp_vcc
	tftp_onofftime
	tftp_pppflow
	tftp_initlog
	tftp_ulog
}

main "$@"

