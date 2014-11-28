#!/bin/bash

. /sbin/autelan_functions.sh
LOCAL_TMP_LOG=/tmp/3g-flow.log
upload_3g_downflow=
upload_3g_upflow=

do_tftp() {
	local LOCAL_TMP_FILE=/tmp/3g-flow-$Time.log
	local PEER_TMP_FILE=/tmp/tftp/log/flow/3g/3g-flow-$Time.log
	local PEER_OPT_PATH=/opt/log/flow/3g

	[ -z "$PEER" ] && PEER=1.0.0.2

	if [ "$Time" ]; then
		/bin/cp $LOCAL_TMP_LOG  $LOCAL_TMP_FILE 2> /dev/null
		if [ -f $LOCAL_TMP_FILE ];then
			echo "$0: /usr/bin/tftp -pl $LOCAL_TMP_FILE -r $PEER_TMP_FILE $PEER" >> $DEBUG_LOG_LOCAL
			/usr/bin/tftp -pl $LOCAL_TMP_FILE -r $PEER_TMP_FILE $PEER
			if [ $? = 0 ];then
				echo "$0: TFTP OK" >> $DEBUG_LOG_LOCAL
				> $LOCAL_TMP_LOG
				echo "$0: DEBUG=on /etc/jsock/jcmd.sh syn mv $PEER_TMP_FILE $PEER_OPT_PATH &" >> $DEBUG_LOG_LOCAL
				DEBUG=on /etc/jsock/jcmd.sh syn mv $PEER_TMP_FILE $PEER_OPT_PATH &
			
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
	get_time
	echo "$Time" > $PPPTMPPATH/endtime

	local STTIME=$(cat $PPPTMPPATH/starttime)
	local EDTIME=$(cat $PPPTMPPATH/endtime)

	JSstring={\"device\":\"cdma\",\"starttime\":\"$STTIME\",\"endtime\":\"$EDTIME\",\"up\":\"${upload_3g_upflow}\",\"down\":\"${upload_3g_downflow}\"}
	echo $JSstring >> $LOCAL_TMP_LOG
}

do_service() {
	local SER_INTVAL=30
	local intval_count=0

	if [ ! -f $PPPTMPPATH/starttime_all ]; then
		sleep 30	
		get_time
		echo "$Time" > $PPPTMPPATH/starttime_all
	else
		get_time
	fi
	echo "$Time" > $PPPTMPPATH/starttime
	
	while [ $? == "0" ]; do
		/bin/sleep $SER_INTVAL
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
	check_interface
	if [ $? == "0" ]; then
		echo "true" > $PPPTMPPATH/access
		get_time
		#echo "$0: DEBUG=on /etc/jsock/jmsg.sh asyn 3g_up {\"date\":\"$Time\"}" >> $DEBUG_LOG_LOCAL
		#DEBUG=on /etc/jsock/jmsg.sh asyn 3g_up {\"date\":\"$Time\"}	
		#/etc/init.d/wifidog stop
		do_service
	fi
}

main "$@"

