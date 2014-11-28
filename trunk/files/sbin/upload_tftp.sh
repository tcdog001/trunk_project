#!/bin/sh

. /sbin/autelan_functions.sh

tftp_vcc() {                 
	local PEER_TMP_PATH=/tmp/tftp/log/vcc
	local PEER_OPT_PATH=/opt/log/vcc
	local LOCAL_ROOT_PATH=/root/vcc
	local line=$(ls -la /root/vcc |wc -l)

	[ -z "${PEER}" ] && PEER=1.0.0.2       
	get_time

	ls ${LOCAL_ROOT_PATH} |grep vcc-quality >/dev/null 2>&1
	if [[ $? = 0 ]]; then
		echo "$0: /usr/bin/tftp -pl ${LOCAL_ROOT_PATH}/vcc-quality-${Time} -r ${PEER_TMP_PATH}/vcc-quality-${Time} ${PEER}" >>${TFTP_LOG}
		for file in $(ls ${LOCAL_ROOT_PATH} |grep vcc-quality)
		do
			/usr/bin/tftp -pl ${LOCAL_ROOT_PATH}/${file} -r ${PEER_TMP_PATH}/${file} ${PEER} 2>/dev/null
			if [[ $? = 0 ]];then
				rm -rf ${LOCAL_ROOT_PATH}/${file}
				echo "$0: ${file} TFTP OK" >>${TFTP_LOG}
				DEBUG=on /etc/jsock/jcmd.sh syn mv ${PEER_TMP_PATH}/* ${PEER_OPT_PATH}/ &
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

	[ -z "${PEER}" ] && PEER=1.0.0.2
	get_time

	ls ${LOCAL_ROOT_PATH} |grep ap-on-off >/dev/null 2>&1
	if [[ $? = 0 ]];then
		echo "$0: /usr/bin/tftp -pl ${LOCAL_ROOT_PATH}/ap-on-off-${Time} -r ${PEER_TMP_PATH}/ap-on-off-${Time} ${PEER}" >>${TFTP_LOG}
		for file in $(ls ${LOCAL_ROOT_PATH} |grep ap-on-off)
		do
			/usr/bin/tftp -p -l ${LOCAL_ROOT_PATH}/${file} -r ${PEER_TMP_PATH}/${file} ${PEER} 2>/dev/null
			if [[ $? = 0 ]];then
				rm -rf ${LOCAL_ROOT_PATH}/${file}
				echo "$0: ${file} TFTP OK" >>${TFTP_LOG}
				DEBUG=on /etc/jsock/jcmd.sh syn mv ${PEER_TMP_PATH}/* ${PEER_OPT_PATH}/ & 
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

	[ -z "${PEER}" ] && PEER=1.0.0.2
	get_time

	ls ${LOCAL_ROOT_PATH} |grep ${STRING} > /dev/null 2>&1
	if [[ $? = 0 ]];then
		for file in $(ls ${LOCAL_ROOT_PATH} |grep ${STRING})
		do
			/usr/bin/tftp -p -l ${LOCAL_ROOT_PATH}/${file} -r ${PEER_TMP_PATH}/${file} ${PEER} 2>/dev/null
			if [[ $? = 0 ]];then
				rm -rf ${LOCAL_ROOT_PATH}/${file}
				echo "$0: ${file} TFTP OK" >>${TFTP_LOG}
				DEBUG=on /etc/jsock/jcmd.sh syn mv ${PEER_TMP_PATH}/* ${PEER_OPT_PATH}/ &
			else
				echo "$0: ${file} TFTP NOK" >>${TFTP_LOG}
			fi
		done
	fi
}

main() {
	local TFTP_LOG=/tmp/.tftp.log

	sleep 15
	tftp_vcc
	tftp_onofftime
	tftp_pppflow
}

main "$@"

