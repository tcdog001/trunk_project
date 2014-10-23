#!/bin/bash

. /sbin/autelan_functions.sh

do_tftp() {
	local LOCAL_TMP_PATH=/tmp
	local PEER_TMP_PATH=/tmp/tftp/log/sys/ap/ulog
	local PEER_OPT_PATH=/opt/log/sys/ap/ulog

	[ -z "$PEER" ] && PEER=1.0.0.2

	if [ "$Time" ]; then
		/bin/cp $LOCAL_TMP_PATH/3g_flow.log  $LOCAL_TMP_PATH/3g_flow-$Time.log 2> /dev/null
		if [ -f $LOCAL_TMP_PATH/3g_flow-$Time.log ];then
			echo "$0: /usr/bin/tftp -pl $LOCAL_TMP_PATH/3g_flow-$Time.log -r $PEER_TMP_PATH/3g_flow.log $PEER" >> $DEBUG_LOG_LOCAL
			/usr/bin/tftp -pl $LOCAL_TMP_PATH/3g_flow-$Time.log -r $PEER_TMP_PATH/3g_flow.log $PEER
			if [ $? = 0 ];then
				echo "$0: TFTP OK" >> $DEBUG_LOG_LOCAL
				echo "" > $LOCAL_TMP_PATH/3g_flow.log
				echo "$0: DEBUG=on /etc/jsock/jcmd.sh syn mv $PEER_TMP_PATH/3g_flow.log $PEER_OPT_PATH &" >> $DEBUG_LOG_LOCAL
				DEBUG=on /etc/jsock/jcmd.sh syn mv $PEER_TMP_PATH/3g_flow.log $PEER_OPT_PATH &
			
			else
				echo "$0: TFTP NOK" >> $DEBUG_LOG_LOCAL
			fi
			/bin/rm -rf "$LOCAL_TMP_PATH/3g_flow-$Time.log" >> $DEBUG_LOG_LOCAL
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

json_string() {
	get_time
	local EDTIME=$Time

	DOWNFLOW=$( /sbin/ifconfig 3g-evdo | /usr/bin/awk -F ' ' '/RX packets/{print $2}' | /usr/bin/awk -F ':' '{print $2}' )
	UPFLOW=$( /sbin/ifconfig 3g-evdo | /usr/bin/awk -F ' ' '/TX packets/{print $2}' | /usr/bin/awk -F ':' '{print $2}' )
	JSstring={\"device\":\"cdma\",\"starttime\":\"$STTIME\",\"endtime\":\"$EDTIME\",\"up\":\"$UPFLOW\",\"down\":\"$DOWNFLOW\"}

	echo $JSstring >> /tmp/3g_flow.log
}

do_service() {
	local SER_INTVAL=300

	while [ $? == "0" ]; do
		/bin/sleep $SER_INTVAL
		json_string
		do_tftp
		check_interface
	done
}

main() {
	check_interface
	if [ $? == "0" ]; then
		echo "true" > /tmp/.ppp/access
		get_time
		echo "$0: DEBUG=on /etc/jsock/jmsg.sh asyn 3g_up {\"date\":\"$Time\"}" >> $DEBUG_LOG_LOCAL
		DEBUG=on /etc/jsock/jmsg.sh asyn 3g_up {\"date\":\"$Time\"}	
		/etc/init.d/wifidog stop
		[ ! -f /tmp/.ppp/starttime ] && echo "$Time" > /tmp/.ppp/starttime 
		echo "$Time" > /tmp/.ppp/endtime
		STTIME=$Time
		do_service
	fi
}

main $@
exit 0

