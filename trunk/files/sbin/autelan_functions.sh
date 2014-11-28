#!/bin/sh

#
# autelan scripts log path
#
#DEBUG_LOG_LOCAL=/tmp/debug.log
DEBUG_LOG_LOCAL=/dev/null
Time=

#
# ppp log path
#
readonly PPPPATH=/root/ppp
readonly DOWNFLOWPATH=$PPPPATH/down_all
readonly UPFLOWPATH=$PPPPATH/up_all
readonly PPPTMPPATH=/tmp/.ppp
readonly DOWNFLOWTMP=$PPPTMPPATH/down_now
readonly UPFLOWTMP=$PPPTMPPATH/up_now

ppp_json_string() {
	local DOWNFLOW=$(cat $DOWNFLOWPATH)
	local UPFLOW=$(cat $UPFLOWPATH)
	local EDTIME=$(cat $PPPTMPPATH/endtime)
	local STTIME=$(cat $PPPTMPPATH/starttime_all)
	local JSstring={\"device\":\"cdma\",\"starttime\":\"$STTIME\",\"endtime\":\"$EDTIME\",\"up\":\"$UPFLOW\",\"down\":\"$DOWNFLOW\"}

	echo $JSstring >> $PPPPATH/3g-flow-$Time.log
	rm -f $UPFLOWPATH $DOWNFLOWPATH
}

get_time() {
	Time=$(date -I "+%F-%H:%M:%S")
	#Time=$(/bin/date -Iseconds | /bin/sed 's/:/-/g;s/T/-/g;s/+0800//g')
	#echo $Time
}

