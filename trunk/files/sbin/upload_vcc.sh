#!/bin/sh

. /sbin/autelan_functions.sh
FILENAME=/tmp/vcc.log

do_tftp() {
	local LOCAL_TMP_PATH=/tmp
	local PEER_TMP_PATH=/tmp/tftp/log/sys/ap/ulog
	local PEER_OPT_PATH=/opt/log/sys/ap/ulog

	[ -z "$PEER" ] && PEER=1.0.0.2
	if [ "$Time" ]; then
		/bin/cp $FILENAME  $LOCAL_TMP_PATH/vcc-$Time.log 2> /dev/null
		if [ -f $LOCAL_TMP_PATH/vcc-$Time.log ];then
			echo "$0: /usr/bin/tftp -pl $LOCAL_TMP_PATH/vcc-$Time.log -r $PEER_TMP_PATH/vcc-quality.log $PEER"
			/usr/bin/tftp -pl $LOCAL_TMP_PATH/vcc-$Time.log -r $PEER_TMP_PATH/vcc-quality.log $PEER
			if [ $? = 0 ];then
				echo "$0: TFTP OK"
				echo "" > $FILENAME
				DEBUG=on /etc/jsock/jcmd.sh syn mv $PEER_TMP_PATH/vcc-quality.log $PEER_OPT_PATH &
			else
				echo "$0: TFTP NOK"
				/bin/cp $FILENAME /root/vcc.log
			fi
			rm -rf $LOCAL_TMP_PATH/vcc-$Time.log
		fi

	else
		exit 1
	fi
}

get_vcc_time() {

	local time=` /bin/cat $FILENAME |/usr/bin/awk -F ',' '{print $1}' | /bin/sed -n '$p' `
	#echo $time
	Time=` echo "$time" |/usr/bin/awk -F '"' '{print $4}' `
	echo "$0: $Time"
}

main() {
	if [ -f "$FILENAME" ];then
		get_vcc_time
		do_tftp
	else
		echo "$0: $FILENAME not found"
	fi	
}

main $@
exit 0
