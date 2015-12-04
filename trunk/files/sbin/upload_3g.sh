#!/bin/bash

. /sbin/autelan_functions.in
LOCAL_TMP_LOG=/tmp/3g-flow.log
upload_3g_downflow=
upload_3g_upflow=

do_tftp() {
	local time=$(get_time)
	local LOCAL_TMP_FILE=/tmp/3g-flow-${time}.log
	local PEER_TMP_FILE=/tmp/tftp/log/flow/3g/3g-flow-${time}.log
	local PEER_OPT_PATH=/opt/log/flow/3g

	[ -z ${PEER} ] && PEER=1.0.0.2

	if [ ${time} ]; then
		/bin/cp $LOCAL_TMP_LOG  $LOCAL_TMP_FILE 2> /dev/null
		if [ -f $LOCAL_TMP_FILE ];then
			echo "$0: /usr/bin/tftp -pl $LOCAL_TMP_FILE -r $PEER_TMP_FILE $PEER" >> $DEBUG_LOG_LOCAL
			/usr/bin/tftp -pl $LOCAL_TMP_FILE -r $PEER_TMP_FILE $PEER
			if [ $? = 0 ];then
				echo "$0: TFTP OK" >> $DEBUG_LOG_LOCAL
				> $LOCAL_TMP_LOG
				echo "$0: /etc/jsock/jcmd.sh syn mv $PEER_TMP_FILE $PEER_OPT_PATH &" >> $DEBUG_LOG_LOCAL
				/etc/jsock/jcmd.sh syn mv $PEER_TMP_FILE $PEER_OPT_PATH &
			
			else
				echo "$0: TFTP NOK" >> $DEBUG_LOG_LOCAL
			fi
			/bin/rm -rf "$LOCAL_TMP_FILE" >> $DEBUG_LOG_LOCAL
		fi

	else
		exit -1
	fi
}

check_interface() {
	local RT=0
	local STRING=$( /sbin/ifconfig 3g-evdo | /bin/grep "Point-to-Point Protoco" )
	[ -z "$STRING" ] && RT=1

	return $RT
}

get_flow() {
	upload_3g_downflow=$(ifconfig 3g-evdo | awk '/RX bytes/{print $2}' | awk -F ':' '{print $2}')
	upload_3g_upflow=$(ifconfig 3g-evdo | awk '/TX bytes/{print $6}' | awk -F ':' '{print $2}')
	echo ${upload_3g_downflow} > $DOWNFLOWTMP
	echo ${upload_3g_upflow} > $UPFLOWTMP
}

json_string() {
	local time=$(get_time)
	echo "${time}" > $PPPTMPPATH/endtime

	local STTIME=$(cat $PPPTMPPATH/starttime)
	local EDTIME=$(cat $PPPTMPPATH/endtime)

	JSstring={\"device\":\"cdma\",\"starttime\":\"$STTIME\",\"endtime\":\"$EDTIME\",\"up\":${upload_3g_upflow},\"down\":${upload_3g_downflow}}
	echo $JSstring >> $LOCAL_TMP_LOG
}

do_service() {
	local SER_INTVAL=30
	local intval_count=0
	local time=$(get_time)

	if [ ! -f $PPPTMPPATH/starttime_all ]; then
		sleep 30
		time=$(get_time)
		echo ${time} > $PPPTMPPATH/starttime_all
	fi
	echo ${time} > $PPPTMPPATH/starttime
	
	while [ $? == "0" ]; do
		/bin/sleep ${SER_INTVAL}
		get_flow
		((intval_count++))
		if [ ${intval_count} -ge 10 ]; then
			json_string
			do_tftp
			intval_count=0
		fi
		check_interface
	done
}

main() {
	local time=""

	check_interface
	if [ $? == "0" ]; then
		echo "true" > $PPPTMPPATH/access
		#time=$(get_time)
		#echo "$0: /etc/jsock/jmsg.sh asyn 3g_up {\"date\":\"${time}\"}" >> $DEBUG_LOG_LOCAL
		#/etc/jsock/jmsg.sh asyn 3g_up {\"date\":\"${time}\"}
		#/etc/init.d/wifidog stop
		do_service
	fi
}

main "$@"

