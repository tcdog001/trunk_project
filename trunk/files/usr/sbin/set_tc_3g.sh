#!/bin/sh
# xiaoh www.linuxbyte.org
 

IDEV="eth0"

#ap 3g download speed
UP="90mbit"
DOWN="90mbit"
#the user 3g download speed
DOWNLOAD="400kbit"
MDOWNLOAD="400kbit"

#the user 3g upload speed
UPLOAD="400kbit"
MUPLOAD="400kbit"

#the user wifi upload speed
UPLOAD_WIFI="4mbit"
MUPLOAD_WIFI="4mbit"
#IP
INET="192.168.0."
 

IPS="2" 
IPE="254"

tc qdisc del dev $IDEV root 2>/dev/null


tc qdisc add dev $IDEV root handle 10: htb default 256

i=$IPS;
while [ $i -le $IPE ]
do

tc class add dev $IDEV parent 10: classid 10:5$i htb rate $DOWNLOAD ceil $MDOWNLOAD
tc filter add dev $IDEV parent 10: prio 100 protocol ip u32 match ip dst $INET$i flowid 10:5$i

tc class add dev $IDEV parent 10: classid 10:3$i htb rate $UPLOAD_WIFI ceil $MUPLOAD_WIFI
tc filter add dev $IDEV parent 10: prio 50 protocol ip u32 match ip dst 192.168.0.1/24 match ip src $INET$i flowid 10:3$i

tc class add dev $IDEV parent 10: classid 10:4$i htb rate $UPLOAD ceil $MUPLOAD
tc filter add dev $IDEV parent 10: prio 100 protocol ip u32 match ip src $INET$i flowid 10:4$i

i=`expr $i + 1`
done
