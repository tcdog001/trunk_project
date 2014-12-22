#!/bin/bash

. /sbin/autelan_functions.sh
LOCAL_GPS_FILE=/tmp/gps.log
REP_INTVAL=30

do_tftp() {
	local time=$(get_time)
	local LOCAL_TMP_FILE=/tmp/gps-${time}.log
	local PEER_TMP_FILE=/tmp/tftp/log/gps/gps-${time}.log
	local PEER_OPT_PATH=/opt/log/gps

	[ -z "$PEER" ] && PEER=1.0.0.2

	if [ ${Status} ] && [ ${time} ]; then
		/bin/cp $LOCAL_GPS_FILE  $LOCAL_TMP_FILE 2> /dev/null
		echo "$0: /usr/bin/tftp -pl $LOCAL_TMP_FILE -r $PEER_TMP_FILE $PEER" >> $DEBUG_LOG_LOCAL
		/usr/bin/tftp -pl $LOCAL_TMP_FILE -r $PEER_TMP_FILE $PEER
		if [ $? = 0 ];then
			echo "$0: TFTP OK" >> $DEBUG_LOG_LOCAL
			> $LOCAL_GPS_FILE
			/etc/jsock/jcmd.sh syn mv $PEER_TMP_FILE $PEER_OPT_PATH &
		else
			echo "$0: TFTP NOK" >> $DEBUG_LOG_LOCAL
		fi
		/bin/rm -rf $LOCAL_TMP_FILE >> $DEBUG_LOG_LOCAL
	else
		exit -1
	fi
}

get_status() {

	Status=$(/bin/cat $LOCAL_GPS_FILE |/usr/bin/awk -F ',' '{print $1}' | /bin/sed -n '$p')
	echo "$0: $Status" >> $DEBUG_LOG_LOCAL
}

do_service() {
	local SER_INTVAL=300

	while :
	do
		/bin/sleep $SER_INTVAL
		get_status
		do_tftp
	done
}

stop_service() {

	local PID=$(/bin/ps | /usr/bin/awk '/rgps/{if($5=="/usr/sbin/rgps"){print $1}}' | /usr/bin/awk '{if(NR==1){print $1}}')
	kill -9 $PID 2> /dev/null
	echo "kill /usr/bin/rgps" >> $DEBUG_LOG_LOCAL

	local PID=$(/bin/ps | /usr/bin/awk '/rgps/{if($5=="rgps"){print $1}}' | /usr/bin/awk '{if(NR==1){print $1}}')
	kill -9 $PID 2> /dev/null
	echo "kill rgps" >> $DEBUG_LOG_LOCAL
}

service_restart() {

	stop_service
	/bin/sleep 1
	/usr/sbin/rgps -d /dev/ttyS1 -l $LOCAL_GPS_FILE -t $REP_INTVAL 1> /dev/null &
	/bin/sleep $REP_INTVAL
}

main() {
	/bin/sleep $REP_INTVAL
	if [ -f $LOCAL_GPS_FILE ];then
		/bin/sleep 5
		get_status
		if [ -z $Status ]; then
			service_restart
		fi
		do_service
	else
		service_restart
		do_service
	fi	
}

main "$@"

