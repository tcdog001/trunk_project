#!/bin/sh

. /sbin/autelan_functions.sh
LOCAL_TMP_PATH=/tmp/.ppp

do_tftp() {
	local LOCAL_TMP_PATH=/tmp
	local PEER_TMP_PATH=/tmp/tftp/log/sys/ap/ulog
	local PEER_OPT_PATH=/opt/log/sys/ap/ulog

	[ -z "$PEER" ] && PEER=1.0.0.2

	/bin/cp $LOCAL_TMP_PATH/3g_drop.log  $LOCAL_TMP_PATH/3g_drop-tmp.log 2> /dev/null
	if [ -f $LOCAL_TMP_PATH/3g_drop-tmp.log ];then
		echo "$0: /usr/bin/tftp -pl $LOCAL_TMP_PATH/3g_drop-tmp.log -r $PEER_TMP_PATH/3g_drop.log $PEER" >> $DEBUG_LOG_LOCAL
		/usr/bin/tftp -pl $LOCAL_TMP_PATH/3g_drop-tmp.log -r $PEER_TMP_PATH/3g_drop.log $PEER
		if [ $? = 0 ];then
			echo "$0: TFTP OK" >> $DEBUG_LOG_LOCAL
			echo "" > $LOCAL_TMP_PATH/3g_drop.log
			echo "$0: DEBUG=on /etc/jsock/jcmd.sh syn mv $PEER_TMP_PATH/3g_drop.log $PEER_OPT_PATH &" >> $DEBUG_LOG_LOCAL
			DEBUG=on /etc/jsock/jcmd.sh syn mv $PEER_TMP_PATH/3g_drop.log $PEER_OPT_PATH &
			
		else
			echo "$0: TFTP NOK" >> $DEBUG_LOG_LOCAL
		fi
		/bin/rm -rf "$LOCAL_TMP_PATH/3g_drop-tmp.log" >> $DEBUG_LOG_LOCAL
	fi
}

json_string() {
	local STTIME EDTIME DBM CSQ DCOUNT RCOUNT
	RCOUNT=` /bin/cat $LOCAL_TMP_PATH/ppp.log  | /bin/grep "Connect script failed" | /usr/bin/wc -l `
	DCOUNT=` /bin/cat $LOCAL_TMP_PATH/ppp.log  | /bin/grep "chat:" | /usr/bin/wc -l `
	ACCESS=` /bin/cat $LOCAL_TMP_PATH/access `
	STTIME=` /bin/cat $LOCAL_TMP_PATH/starttime `
	EDTIME=` /bin/cat $LOCAL_TMP_PATH/endtime `
	DBM=` /usr/sbin/at_ctrl at^hdrcsq | /usr/bin/awk -F ':' '/HDRCSQ/{print $2}' `
	CSQ=` /usr/sbin/at_ctrl at+csq | /usr/bin/awk -F ':' '/CSQ/{print $2}' | /usr/bin/awk -F ',' '{print $1}' `

	JSstring={\"type\":\"CDMA\",\"recordtime\":\"$NOWTIME\",\"starttime\":\"$STTIME\",\"endtime\":\"$EDTIME\",\"dialcount\":\"$DCOUNT\",\"resetcount\":\"$RCOUNT\",\"access\":\"$ACCESS\",\"dbm\":\"$DBM\",\"csq\":\"$CSQ\"}
	echo $JSstring >> /tmp/3g_drop.log
}

do_service() {
	json_string
	do_tftp
}

main() {
	local PID=` /bin/ps | /usr/bin/awk '/upload_3g.sh/{if($5=="{upload_3g.sh}"){print $1}}' | /usr/bin/awk '{if(NR==1){print $1}}' `
	kill -9 $PID 2> /dev/null

	get_time
	echo "$0: DEBUG=on /etc/jsock/jmsg.sh asyn 3g_down {\"date\":\"$Time\"}" >> $DEBUG_LOG_LOCAL
	DEBUG=on /etc/jsock/jmsg.sh asyn 3g_down {\"date\":\"$Time\"}
	/etc/init.d/wifidog start
	NOWTIME=$Time
	do_service
	echo "false" > $LOCAL_TMP_PATH/access
}

main $@
exit 0

