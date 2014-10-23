#!/bin/sh

#DEBUG_LOG_LOCAL=/tmp/debug.log
DEBUG_LOG_LOCAL=/dev/null

get_time() {
	Time=` /bin/date -Iseconds | /bin/sed 's/:/-/g;s/T/-/g;s/+0800//g' `
	#echo $Time
}

