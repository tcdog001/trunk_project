#!/bin/sh
# xiaoh www.linuxbyte.org
 

IDEV="eth0"

#ap 3g download speed
UP="90mbit"
DOWN="90mbit"
#the user 3g download speed
DOWNLOAD="400kbit"
MDOWNLOAD="400kbit"

 
#IP
INET="192.168.0."
 

IPS="2" 
IPE="254"

tc qdisc del dev $IDEV root 2>/dev/null


tc qdisc add dev $IDEV root handle 10: htb default 256

i=$IPS;
while [ $i -le $IPE ]
do

tc class add dev $IDEV parent 10: classid 10:2$i htb rate $DOWNLOAD ceil $MDOWNLOAD
tc filter add dev $IDEV parent 10: prio 100 protocol ip u32 match ip dst $INET$i flowid 10:2$i

i=`expr $i + 1`
done
