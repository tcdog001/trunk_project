#!/bin/bash

#
# autelan scripts log path
#
#DEBUG_LOG_LOCAL=/tmp/debug.log
DEBUG_LOG_LOCAL=/dev/null

#
# ppp log path
#
readonly PPPPATH=/root/ppp
readonly DOWNFLOWPATH=${PPPPATH}/down_all
readonly UPFLOWPATH=${PPPPATH}/up_all
readonly PPPTMPPATH=/tmp/.ppp
readonly DOWNFLOWTMP=${PPPTMPPATH}/down_now
readonly UPFLOWTMP=${PPPTMPPATH}/up_now
readonly DMESGPATH=/root/log/init
readonly SYSLOGDPATH=/root/log/ulog
readonly LASTSYNTIME=/root/log/ulog/timesyn
readonly SYNTIME=/tmp/timesyn

ppp_json_string() {
	local time=$(get_time)
	local DOWNFLOW=$(cat ${DOWNFLOWPATH} 2>> ${DEBUG_LOG_LOCAL})
	local UPFLOW=$(cat ${UPFLOWPATH} 2>> ${DEBUG_LOG_LOCAL})
	local EDTIME=$(cat ${PPPTMPPATH}/endtime 2>> ${DEBUG_LOG_LOCAL})
	local STTIME=$(cat ${PPPTMPPATH}/starttime_all 2>> ${DEBUG_LOG_LOCAL})
	local JSstring={\"device\":\"cdma\",\"starttime\":\"${STTIME}\",\"endtime\":\"${EDTIME}\",\"up\":\"${UPFLOW}\",\"down\":\"${DOWNFLOW}\"}

	echo ${JSstring} >> ${PPPPATH}/3g-flow-${time}.log
	rm -f ${UPFLOWPATH} ${DOWNFLOWPATH}
}

get_time() {
	local Time=$(date -I "+%F-%H:%M:%S")
	#Time=$(/bin/date -Iseconds | /bin/sed 's/:/-/g;s/T/-/g;s/+0800//g')
	echo ${Time}
}

#
# $1: file, content time like 2014-12-10 15:37:37
# output time like 2014-12-10-15:37:37
#
modify_time() {
	local file="$1"
	sed -i 's/ /-/g' ${file}
}

get_syntime() {
	local file=${SYNTIME}
	local time=""

	if [ -f ${file} ]; then
		modify_time ${file}
		cat ${file}
	else
		time=$(get_time)
		echo ${time}
	fi
}

get_last_syntime() {
	local file=${LASTSYNTIME}
	local time=""

	if [ -f ${file} ]; then
		modify_time ${file}
		cat ${file}
	else
		time=$(get_time)
		echo ${time}
	fi
}

save_init_log() {
	local time=$(get_syntime)
	local path=${DMESGPATH}

	# save init log
	dmesg > ${path}/init-${time}
}

save_last_syslog() {
	local time=$(get_last_syntime)
	local file=${SYSLOGDPATH}/messages
	
	# save last syslogd message
	[[ -f ${file} ]] && mv ${file} ${SYSLOGDPATH}/ulog-${time}
}

restart_prepare() {
	local time=$(get_time)
	local cmd="/etc/jsock/jmsg.sh asyn acc_off {\"date\":\"${time}\"}"
	local ontime_file=/root/onoff/ap-on

	acc_power_off.sh &
	upload_vcc.sh &
	sleep 5

	echo "$0: ${cmd}"
	${cmd} &

	ppp_json_string

	mv /tmp/log/messages ${SYSLOGDPATH}/
	mv ${SYNTIME} ${SYSLOGDPATH}/
	sleep 5
}

#
# $1: R_path
# $2: M_tmppath
# $3: M_path
# $4: M_ip
# $5: file
# $6: filerename
#
tftp_RtoM() {
	local R_path="$1"
	local M_tmppath="$2"
	local M_path="$3"
	local M_ip="$4"
	local file="$5"
	local filerename="$6"

	tftp -p -l ${R_path}/${file} -r ${M_tmppath}/${filerename} ${M_ip} 2>> ${DEBUG_LOG_LOCAL}
	if [[ $? = 0 ]];then
		echo "$0: ${file} TFTP OK" 2>> ${DEBUG_LOG_LOCAL}
		/etc/jsock/jcmd.sh syn mv ${M_tmppath}/${filerename} ${M_path}/ &
		rm -rf ${R_path}/${file}
		return 0
	else
		echo "$0: ${file} TFTP NOK" 2>> ${DEBUG_LOG_LOCAL}
		return 1
	fi
}

get_3g_net() {
    local code=$(cat /root/ppp/iccid 2> /dev/null | awk -F '' '{print $5$6}')
    local net=""
    
    [[ "${code}" = "01" ]] && net=WCDMA
    [[ "${code}" = "03" ]] && net=CDMA2000
    [[ "${code}" = "00" ]] && net=GSM/TD-SCDMA
    [[ "${code}" = "02" ]] && net=GSM/TD-SCDMA
    [[ "${code}" = "07" ]] && net=TD-SCDMA
    echo ${net}
}

