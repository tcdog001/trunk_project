#!/bin/sh
# xiaoh www.linuxbyte.org
 

ODEV_2="wlan0-1"
ODEV_5="wlan1"

#the user local download speed
LOCALLOAD_2="4mbit"
MLOCALLOAD_2="4mbit"
LOCALLOAD_5="4mbit"
MLOCALLOAD_5="4mbit" 
#IP
INET="192.168.0."
 

IPS="2" 
IPE="254"

tc qdisc del dev $ODEV_2 root 2>/dev/null
tc qdisc del dev $ODEV_5 root 2>/dev/null

tc qdisc add dev $ODEV_2 root handle 10: htb default 256
tc qdisc add dev $ODEV_5 root handle 10: htb default 256


i=$IPS;
while [ $i -le $IPE ]
do

tc class add dev $ODEV_2 parent 10: classid 10:2$i htb rate $LOCALLOAD_2 ceil $MLOCALLOAD_2
tc filter add dev $ODEV_2 parent 10: prio 100 protocol ip u32 match ip src 192.168.0.1/24 match ip dst $INET$i flowid 10:2$i

tc class add dev $ODEV_5 parent 10: classid 10:6$i htb rate $LOCALLOAD_5 ceil $MLOCALLOAD_5
tc filter add dev $ODEV_5 parent 10: prio 100 protocol ip u32 match ip src 192.168.0.1/24 match ip dst $INET$i flowid 10:6$i

i=`expr $i + 1`
done