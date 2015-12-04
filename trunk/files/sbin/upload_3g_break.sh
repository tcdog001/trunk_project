#!/bin/bash

. /sbin/autelan_functions.in
LOCAL_TMP_LOG=/tmp/3g-flow.log

do_tftp() {
	local time=$(get_time)
	local LOCAL_TMP_FILE=/tmp/3g-flow-${time}.log
	local PEER_TMP_FILE=/tmp/tftp/log/flow/3g/3g-flow-${time}.log
	local PEER_OPT_PATH=/opt/log/flow/3g

	[ -z "$PEER" ] && PEER=1.0.0.2

	if [ "${time}" ]; then
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

json_string() {
	local time=$(get_time)
	#local flowstring=$(cat /tmp/.ppp/ppp.log | sed -n '/, received/p' | sed -n '$p')
	local DOWNFLOW=$(cat $DOWNFLOWTMP)
	local UPFLOW=$(cat $UPFLOWTMP)
	local STTIME=$(/bin/cat $PPPTMPPATH/starttime)
	local EDTIME=${time}

	JSstring={\"device\":\"cdma\",\"starttime\":\"$STTIME\",\"endtime\":\"$EDTIME\",\"up\":\"$UPFLOW\",\"down\":\"$DOWNFLOW\"}
	echo $JSstring >> $LOCAL_TMP_LOG
}

do_service() {
	json_string
	do_tftp
}

do_calc() {
	[ ! -f "$UPFLOWPATH" ] && mkdir -p $PPPPATH
	local downflow_now=$(cat $DOWNFLOWTMP)
	local upflow_now=$(cat $UPFLOWTMP)
	local downflow_all=$(cat $DOWNFLOWPATH)
	local upflow_all=$(cat $UPFLOWPATH)

	upflow_all=$((upflow_all+upflow_now))
	downflow_all=$((downflow_all+downflow_now))

	echo $upflow_all > $UPFLOWPATH
	echo $downflow_all > $DOWNFLOWPATH
	echo "$0: up_now:$upflow_now, down_now:$downflow_now, up_all:$upflow_all, down_all:$downflow_all" >> $DEBUG_LOG_LOCAL
}

main() {
	local time
	local PID=$(/bin/ps | /usr/bin/awk '/upload_3g.sh/{if($5=="{upload_3g.sh}"){print $1}}' | /usr/bin/awk '{if(NR==1){print $1}}')
	kill -9 $PID 2> /dev/null

	do_calc
	#time=$(get_time)
	#echo "$0: /etc/jsock/jmsg.sh asyn 3g_down {\"date\":\"${time}\"}" >> $DEBUG_LOG_LOCAL
	#/etc/jsock/jmsg.sh asyn 3g_down {\"date\":\"${time}\"}
	#/etc/init.d/wifidog start
	do_service
	echo "false" > $PPPTMPPATH/access
	rm -f $DOWNFLOWTMP $UPFLOWTMP
}

main "$@"

